#pragma once
#include <filesystem>
#include <unordered_map>
#include <vector>
#include <string>
#include "registry.h"
#include "yaml-cpp/yaml.h"

#ifdef _WIN32
#include <windows.h>
#endif
#ifdef __linux__
#include <unistd.h>
#include <limits.h>
#endif
#ifdef __APPLE__
#include <mach-o/dyld.h>
#endif

inline int EMPTY_SETTING = -0x3f3f;

template<typename T>
inline T cfg_get(YAML::Node n){
    if(!n)
        throw std::runtime_error("YAML Node not defined");
    if(!n.IsScalar())
        throw std::runtime_error("YAML Node is not a scalar");
    T v;
    if(!YAML::convert<T>::decode(n,v))
                throw std::runtime_error(std::string("expected ") + typeid(T).name()
                                 + ", got \"" + n.Scalar() + "\"");
    return v;
}

class AppConfig{
    public:
        AppConfig() = default;
        AppConfig(const std::filesystem::path& path){
            if(!std::filesystem::exists(path)){
                throw std::runtime_error("Config File \"" + path.string() + "\" dosn't exists");
            }
            YAML::Node content;
            try{
                content  = YAML::LoadFile(path.string());
            }catch(const std::exception& e){
                throw std::runtime_error(std::string("Failed to build YAML Node: ") + e.what());
            }
            if(!content)
                throw std::runtime_error("Failed to load config file \"" + path.string() + "\"");
            YAML::Node exclude = content["exclude"];
            YAML::Node listener_config =content["listener"];
            if(listener_config){
                if(exclude && exclude.IsMap()){
                    if(exclude["hidden"].IsScalar())
                        _settings["exclude_hidden"] = exclude["hidden"].as<bool>();//由于yaml节点不存在时转换布尔为false，所以直接赋值
                    if(exclude["files"] && exclude["files"].IsSequence())
                        for(const YAML::Node node : exclude["files"])\
                            _exclude_files.push_back(node.as<std::string>());
                    if(exclude["dirs"] && exclude["dirs"].IsSequence())
                        for(const YAML::Node node : exclude["dirs"])
                            _exclude_dirs.push_back(node.as<std::string>());
                    if(exclude["patterns"] && exclude["patterns"].IsSequence())
                        for(const YAML::Node node : exclude["patterns"])
                            _exclude_patterns.push_back(node.as<std::string>());
                }
                if(listener_config.IsMap()){
                    YAML::Node enabled = listener_config["enabled"];
                    YAML::Node lms = listener_config["latency_ms"]; // 文件事件进入worker线程前的等待
                    YAML::Node dms = listener_config["debounce_ms"];//文件系统事件合并范围
                    if(enabled && lms && dms && enabled.IsScalar() && lms.IsScalar() && dms.IsScalar()){
                        try{
                            _settings["listener_enabled"] = cfg_get<bool>(enabled);
                            _settings["listener_latency_ms"] = cfg_get<int>(lms);
                            _settings["listener_debounce_ms"] = cfg_get<int>(lms);
                        }catch(const std::exception& e){
                            throw std::runtime_error(std::string("Invailed scalar type: ") + e.what());
                        }
                    }else{
                        throw std::runtime_error("Missing required field \"listener\" in config file: " + path.string());
                    }
                }
            }else{
                throw std::runtime_error("Missing required fields in config file: " + path.string());
            }
        }
    
        int32_t get_setting(const std::string& str){
            if(_settings.find(str) != _settings.end())
                return _settings.at(str);
            else return EMPTY_SETTING;
        }
    private:
        std::vector<std::filesystem::path> _repositories;
        std::vector<std::string> _exclude_files;
        std::vector<std::string> _exclude_dirs;
        std::vector<std::string> _exclude_patterns;
        std::unordered_map<std::string,int32_t> _settings;
};
struct Context{
    reg::Registries registries;
    AppConfig config;
};

inline std::filesystem::path fa_get_exe_path(){
    std::filesystem::path path;
    #ifdef _WIN32
    char buffer[MAX_PATH];
    GetModuleFileNameA(NULL,buffer,MAX_PATH);
    path = std::filesystem::path(buffer).parent_path();
    #endif
    #ifdef __linux__
    char buffer[PATH_MAX];
    ssize_t len = readlink("/proc/self/exe",buffer,sizeof(buffer) -1);
    if(len != -1){
        buffer[len] = '\0';
        path = std::filesystem::path(buffer).parent_path();
    }
    #endif
    #ifdef __APPLE__
    char buffer[PATH_MAX];
    uint32_t size = sizeof(buffer);
    if(_NSGetExecutablePath(buffer,&size) == 0){
        path = std::filesystem::path(buffer).parent_path();
    }
    #endif
    return path;
}