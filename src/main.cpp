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
#include "app.h"

#include "listener.h"



int main(int argc, char* argv[]) {
    (void)argc;
    (void)argv;

    LOGGER.info("THIS PROGRAM IS NOT A RELEASE VERSION , ALSO NOT A USABLE VERSION");
    try{
        Context ctx{
            reg::Registries{},AppConfig(fa_get_exe_path() / ".config.yaml")
        };
        reg::fa_register_builtin_types(ctx);
        reg::fa_register_builtin_repo_templates(ctx);

        std::filesystem::path root = "/Users/zyhfunny/Documents/VSC_PROJ/FileArchiver/tests/test_repo";
        std::filesystem::path create_root = "/Users/zyhfunny/Documents/VSC_PROJ/FileArchiver/tests/new_repo";
        
        //fa_create_repo(create_root,"test",std::vector<std::string>(),std::vector<std::filesystem::path>(),ctx.registries.get_registry<repo::RepoTemplate>().get_type("default"));
        repo::Repository repo = fa_load_repository(root,ctx);
        ctx.registries.get_registry<repo::Repository>().register_t(repo.manifest.get_name(),repo);

        fa_start_fs_listener(ctx);
    }catch(const std::exception& e){
        LOGGER.error(e.what());
    }
}


