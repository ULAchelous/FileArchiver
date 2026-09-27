#include <filesystem>
#include <fstream>
#include <iostream>
#include <string>
#include <vector>
#include "mathf.h"
#include "renamer.h"
#include "data/repo.h"
#include "registry.h"
#include "data/registry_builtin.h"
#include "archiver/archive.h"
#include "archiver/scanner.h"

#include "listener.h"

reg::Registries registries;
YAML::Node config;


void fa_load_config(std::filesystem::path config_file_path){
    std::ofstream file(config_file_path,std::ios::app);
    if(file){
        
    }else{
        throw std::runtime_error("Failed to open config file");
    }
}

int main(int argc, char* argv[]) {
    (void)argc;
    (void)argv;

    LOGGER.info("NOTICE: THIS PROGRAM IS NOT A RELEASE VERSION , ALSO SO NOT A USABLE VERSION");
    try{
        reg::fa_register_builtin_types(registries);
        reg::fa_register_builtin_repo_templates(registries);

        std::filesystem::path root = "/Users/zyhfunny/Documents/VSC_PROJ/FileArchiver/tests/test_repo";
        std::filesystem::path create_root = "/Users/zyhfunny/Documents/VSC_PROJ/FileArchiver/tests/new_repo";
        
        //fa_create_repo(create_root,"test",std::vector<std::string>(),std::vector<std::filesystem::path>(),registries.get_registry<repo::RepoTemplate>().get_type("default"));
        repo::Repository repo = fa_load_repository(root,&registries);
        registries.get_registry<repo::Repository>().register_t(repo.manifest.get_name(),repo);

        fa_start_fs_listener(&registries);
    }catch(const std::exception& e){
        LOGGER.error(e.what());
    }
}


