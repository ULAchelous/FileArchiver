#include<iostream>
#include<filesystem>
#include "data/file.h"
#include "archiver/scanner.h"
#include "logger.h"
#include <string>
#include "registry.h"
#include "app.h"
#include "data/repo.h"
#include <vector>

std::vector<File> fa_scan(const std::filesystem::path& path, const repo::Repository& repo, Context& ctx){
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
            
            const FileType* type;
            try{
                type = fa_get_file_type(entry.path(),ctx);
            }catch(const std::exception& e){
                LOGGER.error("Failed to resolve file type: " + std::string(e.what()));
                return files;
            }

            LOGGER.info("Resolved type for '" + entry.path().string() + "' -> '" + type->id + "'");
            File file(entry.path(), entry.path().stem().string(), ext, type);
            files.push_back(file);
        }
    }

    LOGGER.info("Scan complete: collected " + std::to_string(files.size()) + " files");
    return files;
}