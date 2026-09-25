#include <filesystem>
#include <iostream>
#include <string>
#include "mathf.h"
#include "renamer.h"
#include "data/repo.h"
#include "registry.h"
#include "data/registry_builtin.h"
#include "archiver/archive.h"
#include "archiver/scanner.h"

int main(int argc, char* argv[]) {
    (void)argc;
    (void)argv;

    try {
        reg::Registries registries;
        reg::fa_register_builtin_types(registries);
        std::filesystem::path root = "/Users/zyhfunny/Documents/VSC_PROJ/FileArchiver/tests/test_repo";
        repo::Repository repo = fa_load_repository(root, &registries);
        fa_archive(fa_scan(root,repo, &registries), repo);
    } catch (const std::exception& e) {
        std::cerr << "Repository/registry smoke test failed: " << e.what() << std::endl;
        return 1;
    }
}


