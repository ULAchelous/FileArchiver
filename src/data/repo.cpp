#include<iostream>
#include<fstream>
#include "data/repo.h"
#include "data/file.h"
#include "logger.h"
#include "registry.h"
#include "app.h"

#if defined(_WIN32)
#include <shlobj.h>
#include <windows.h>
#elif defined(__linux__) || defined(__APPLE__)
#include <pwd.h>
#include <unistd.h>
#endif
using namespace repo;

std::string get_home(){
#if defined(_WIN32)
    PWSTR path = nullptr;
    if (SUCCEEDED(SHGetKnownFolderPath(FOLDERID_Profile, 0, nullptr, &path))) {
        int sz = WideCharToMultiByte(CP_UTF8, 0, path, -1, nullptr, 0, nullptr, nullptr);
        std::string r(sz-1, '\0');
        WideCharToMultiByte(CP_UTF8, 0, path, -1, &r[0], sz, nullptr, nullptr);
        CoTaskMemFree(path);
        return r;
    }
    return std::getenv("USERPROFILE") ?: "";
#elif defined(__linux__) || defined(__APPLE__)
    struct passwd* pw = getpwuid(getuid());
    if (pw && pw->pw_dir) return pw->pw_dir;
    return std::getenv("HOME") ?: "";
#else
    return std::getenv("HOME") ?: "";
#endif
}

Manifest::Manifest(const std::filesystem::path& source_file,Context& ctx):_source_file(source_file),_ctx(&ctx){
    LOGGER.info("Loading repository manifest: " + source_file.string());
    if(!std::filesystem::exists(source_file)){
        throw std::runtime_error("Manifest file does not exist: " + source_file.string());
    }
    YAML::Node config = YAML::LoadFile(source_file.string());
    _structure = config["structure"];
    YAML::Node exclude = config["repo"]["exclude"];
    YAML::Node sources = config["repo"]["sources"];
    YAML::Node name = config["repo"]["name"];
    YAML::Node log = config["log"];
    if(_structure && exclude && sources){
        if(!exclude.IsSequence() || !sources.IsSequence())
            throw std::runtime_error("Incorrect YAML type of \"exclude\" or \"sources\"");
        for(const auto& item : exclude){
            _exclude.push_back(item.as<std::string>());
        }
        for(const auto& item : sources){
            std::string item_str = item.as<std::string>();
            if(item_str.rfind("~/",0) == 0)
                item_str = get_home() + item_str.substr(1,item_str.length() -1);
            bool flag = false;
            for(int i=0;i<static_cast<int>(_sources.size())-1;i++){
                if(fa_path_contains(item_str,_sources[i]) || fa_path_contains(_sources[i],item_str)){\
                    flag=true;
                }
            }
            if(flag) continue;
            LOGGER.info(item_str);
           _sources.push_back(item_str);
        }
        if(log && log["file"] && log["file"].IsScalar()){
            _log_file = log["file"].as<std::string>();
        }
        if(name && name.IsScalar())
            _name = name.as<std::string>();
        LOGGER.info("Manifest parsed: repo='" + _name + "', sources=" + std::to_string(_sources.size()) + ", exclusions=" + std::to_string(_exclude.size()));
        _recursion_structure(_structure,"");
    }else{
        throw std::runtime_error("Missing required fields in manifest file: " + source_file.string());
    }
}

Manifest::~Manifest() = default;
//{
    // if(_ctx == nullptr) return;
    // for(const std::filesystem::path& str : _sources)
    //     _ctx->registries.get_registry<Repository>(). ;
//};

void Manifest::_recursion_structure(YAML::Node current,std::string name){
    if(!_structure.IsMap())
        throw std::runtime_error("Invalid structure format in manifest file: " + _source_file.string());
    if(current["types"]){
        for(const auto& iter : current["types"]){
            std::string type_str = iter.as<std::string>();
            const FileType* type = _ctx->registries.get_registry<FileType>().get_type(type_str);
            if(type == nullptr && _ctx->registries.get_registry<FileType>().get_types_by_ext(type_str).empty() && type_str[0] == '.'){//判断是否为扩展名且未被注册
                try{
                    _ctx->registries.get_registry<FileType>().register_t(type_str,FileType(type_str,{type_str},nullptr));
                }catch(const std::exception& e){
                    LOGGER.error(e.what());
                }
                type = _ctx->registries.get_registry<FileType>().get_type(type_str);
            }
            if(type != nullptr && name != ""){
                LOGGER.info("Mapping type '" + std::string(type->id) + "' -> directory '" + name + "'");
                _type_to_dir[type] = name;
            }
        }
    }
    for(const auto& iter : current){
        if(iter.second.IsMap()){
            try{
                _recursion_structure(iter.second,name != "" ? name + "/" + iter.first.as<std::string>() : iter.first.as<std::string>());
            }catch(const std::exception e){
                LOGGER.error(std::string(e.what()));
            }
        }
    }
}

std::filesystem::path get_cfg_file(const std::filesystem::path& root_path){
    std::vector<std::filesystem::path> candidates = {
        root_path / ".manifest.yaml",
        root_path / ".manifest.yml"
    };

    for(const auto& candidate : candidates){
        if(std::filesystem::exists(candidate)){
            return candidate;
        }
    }
    return "";
}

std::string Manifest::get_target_dir(const File& file) const{
    if(_type_to_dir.find(file.type) == _type_to_dir.end())
        throw std::runtime_error("File type not found in manifest: " + std::string(file.type->id));
    return _type_to_dir.at(file.type);
}

repo::Repository fa_load_repository(const std::filesystem::path& repo_path,Context& ctx){
    LOGGER.info("Loading repository from: " + repo_path.string());
    if(!std::filesystem::exists(repo_path)){
        throw std::runtime_error("Repository path does not exist: " + repo_path.string());
    }

    std::filesystem::path config_path = get_cfg_file(repo_path);

    if(config_path.empty()){
        throw std::runtime_error("No repository manifest/config file found in: " + repo_path.string());
    }

    LOGGER.info("Repository config found: " + config_path.string());
    repo::Manifest manifest = Manifest(config_path, ctx);
    LOGGER.set_log_file(repo_path / manifest.get_log_file());
    return Repository(repo_path, manifest);
}

void fa_create_repo(const std::filesystem::path& repo_path,const std::string& name,const std::vector<std::string>& exclude,const std::vector<std::filesystem::path> sources,const RepoTemplate* repo_template){
    
    YAML::Node d_exclude = YAML::Load(repo_template->exclude);
    YAML::Node d_sources = YAML::Load(repo_template->sources);
    if(repo_template->structure.empty() || repo_template->exclude.empty() || repo_template->sources.empty() || repo_template->log_file.empty())
        throw std::runtime_error("Missing required fields in repo template \"" + repo_template->name + "\"");
    if(!d_exclude.IsSequence() || !d_sources.IsSequence())
        throw std::runtime_error("Incorrect YAML type of field \"exclude\" or \"sources\"");
    std::filesystem::path config_file = get_cfg_file(repo_path);
    if(config_file.empty())
        throw std::runtime_error("Unable to find config file");
    if(std::filesystem::exists(config_file))
        std::filesystem::remove(config_file);
    std::ofstream file(config_file,std::ios::app);
    YAML::Node content;
    if(!file)
        throw std::runtime_error("Failed to create Manifest file");
    content["structure"] = YAML::Load(repo_template->structure);
    if(name != "") content["repo"]["name"] = name;
    content["repo"]["exclude"] = YAML::Node();
    if(!exclude.empty()) 
        for(const std::string& str : exclude)
            content["repo"]["exclude"].push_back(str);
    else
        content["repo"]["exclude"] = d_exclude["exclude"];
    if(!sources.empty()) 
        for(const std::filesystem::path& src : sources)
            content["repo"]["sources"].push_back(src.string());
    else
        content["repo"]["sources"] = d_sources["sources"];

    content["log"]["file"] = repo_template->log_file.string();
    
    file << YAML::Dump(content);
    LOGGER.info("Created repo manifest file");
}
