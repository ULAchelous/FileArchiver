#include<iostream>
#include "data/repo.h"
#include "data/file.h"
#include "logger.h"
using namespace repo;

Manifest::Manifest(const std::filesystem::path& source_file,reg::Registries* registries):_source_file(source_file),_registries(registries){
    LOGGER.info("Loading repository manifest: " + source_file.string());
    if(!std::filesystem::exists(source_file)){
        LOGGER.error("Manifest file does not exist: " + source_file.string());
        return;
    }
    try{
        YAML::Node config = YAML::LoadFile(source_file.string());
        _structure = config["structure"];
        YAML::Node exclude = config["repo"]["exclude"];
        YAML::Node sources = config["repo"]["sources"];
        YAML::Node name = config["repo"]["name"];
        YAML::Node log = config["log"];
        if(_structure && exclude && sources){
            for(const auto& item : exclude){
                _exclude.push_back(item.as<std::string>());
            }
            for(const auto& item : sources){
                _sources.push_back(item.as<std::string>());
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
    }catch(const std::exception& e){
        throw e;
    }
}

void Manifest::_recursion_structure(YAML::Node current,std::string name){
    if(!_structure.IsMap())
        throw std::runtime_error("Invalid structure format in manifest file: " + _source_file.string());
    if(current["types"]){
        for(const auto& iter : current["types"]){
            std::string type_str = iter.as<std::string>();
            const FileType* type = _registries->get_registry<FileType>().get_type(type_str);
            if(type == nullptr && type_str[0] == '.'){
                try{
                    _registries->get_registry<FileType>().register_t(type_str,FileType(type_str,{type_str},nullptr));
                }catch(const std::exception& e){
                    LOGGER.error(e.what());
                }
                type = _registries->get_registry<FileType>().get_type(type_str);
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

std::string Manifest::get_target_dir(const File& file) const{
    if(_type_to_dir.find(file.type) == _type_to_dir.end())
        throw std::runtime_error("File type not found in manifest: " + std::string(file.type->id));
    return _type_to_dir.at(file.type);
}

repo::Repository fa_load_repository(const std::filesystem::path& repo_path,reg::Registries* registries){
    LOGGER.info("Loading repository from: " + repo_path.string());
    if(!std::filesystem::exists(repo_path)){
        throw std::runtime_error("Repository path does not exist: " + repo_path.string());
    }

    std::filesystem::path config_path;
    std::vector<std::filesystem::path> candidates = {
        repo_path / ".manifest.yaml",
        repo_path / ".manifest.yml"
    };

    for(const auto& candidate : candidates){
        if(std::filesystem::exists(candidate)){
            config_path = candidate;
            break;
        }
    }

    if(config_path.empty()){
        throw std::runtime_error("No repository manifest/config file found in: " + repo_path.string());
    }

    LOGGER.info("Repository config found: " + config_path.string());
    return Repository(repo_path, Manifest(config_path, registries));
}
