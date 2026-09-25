#include<iostream>
#include<filesystem>
#include "archiver/archive.h"
#include "data/file.h"
#include "logger.h"
#include "data/repo.h"

void fa_archive(const std::vector<File>& pendding_files,const repo::Repository& repo){
    LOGGER.info("archive loop start");
    for(const File& file : pendding_files){
        if(std::filesystem::exists(file.path)){
            fa_archive_one(repo, file);
        }else{
            LOGGER.error("File does not exist: " + file.path.string());
        }
    }
}

void fa_archive_one(const repo::Repository& repo,const File& file){
    std::filesystem::path target;
    try{
        target = repo.root_dir / repo.manifest.get_target_dir(file) / file.path.filename();
    }catch(const std::exception& e){
        LOGGER.error(e.what());
        return;
    }
    std::error_code ec = fa_move_file(file.path, target);
    if(ec){
        LOGGER.error("Error moving file: " + file.path.string(), ec);
    }else{
        LOGGER.info("Moved file: " + file.path.string() + " to " + target.string());
    }
}