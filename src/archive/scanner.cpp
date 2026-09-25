#include<iostream>
#include<filesystem>
#include "data/file.h"
#include "archiver/scanner.h"
#include "logger.h"
#include <string>
#include "registry.h"
#include "data/repo.h"
#include <vector>

std::vector<File> fa_scan(const std::filesystem::path& path, const repo::Repository& repo, reg::Registries* registries){
    std::vector<File> files;
    LOGGER.info("Scanning directory: " + path.string());

    std::error_code ec;
    std::filesystem::directory_iterator dir_iterator(path, ec);
    if(ec){
        LOGGER.error("Failed to scan directory: " + path.string(), ec);
        return files;
    }

    for(const auto& entry : dir_iterator) {
        if(entry.is_regular_file()) {
            const std::vector<std::string>& excluded = repo.manifest.get_exclude();
            if(std::find(excluded.begin(),excluded.end(),entry.path().stem().string() + entry.path().extension().string()) != excluded.end())
                continue;
            std::string ext = entry.path().extension().string();
            LOGGER.info("Processing file: " + entry.path().string() + ", ext=" + ext);


            std::vector<FileType*> types;
            try{
                types = registries->get_registry<FileType>().get_types_by_ext(ext);
            }catch(const std::exception& e){
                try{
                    registries->get_registry<FileType>().register_t(ext,FileType(ext,{ext},nullptr));
                }catch(const std::exception& e){
                    LOGGER.error(e.what());
                    return files;
                }
                types = registries->get_registry<FileType>().get_types_by_ext(ext);
            }

            const FileType* type = types[0];
            for(const FileType* t : types){
                if(t->classify_func != nullptr && t->classify_func(entry.path().string().c_str())){
                    type = t;
                    break;
                }
            }

            LOGGER.info("Resolved type for '" + entry.path().string() + "' -> '" + type->id + "'");
            File file(entry.path(), entry.path().stem().string(), ext, type);
            files.push_back(file);
        }
    }

    LOGGER.info("Scan complete: collected " + std::to_string(files.size()) + " files");
    return files;
}