#include <string>
#include <vector>
#include <filesystem>
#include "listener.h"
#include "data/repo.h"
#include "data/file.h"
#include "logger.h"
#include "archiver/archive.h"


void fa_fs_event_holder(FSEvent event,const std::filesystem::path& path,repo::Repository* repo){
    for(const std::filesystem::path& p : repo->manifest.get_sources()){
        if(fa_path_contains(p,path)){
            switch(event){
                case FSEvent::REMOVED:
                    fa_fs_on_removed(path,repo);
                    break;
                case FSEvent::RENAMED:
                    fa_fs_on_renamed(path,repo);
                    break;
                case FSEvent::CREATED:
                    fa_fs_on_created(path,repo);
                    break;
                case FSEvent::MODIFiED:
                    fa_fs_on_modified(path,repo);
                    break;
                default:
                    break;
            }
        }
    }

}
void fa_fs_on_created(const std::filesystem::path& path,const repo::Repository* repo){
    LOGGER.info("FS event: repo: \"" + repo->manifest.get_name() + "\" created file \"" + path.string() + "\"");
    File file(path,path.stem().string(),path.extension().string(),fa_get_file_type(path,repo->manifest.get_registries()));
}
void fa_fs_on_renamed(const std::filesystem::path& path,const repo::Repository* repo){
    LOGGER.info("FS event: repo: \"" + repo->manifest.get_name() + "\" renamed file \"" + path.string() + "\"");
}
void fa_fs_on_removed(const std::filesystem::path& path,const repo::Repository* repo){
    LOGGER.info("FS event: repo: \"" + repo->manifest.get_name() + "\" removed file \"" + path.string() + "\"");
}
void fa_fs_on_modified(const std::filesystem::path& path,const repo::Repository* repo){
    LOGGER.info("FS event: repo: \"" + repo->manifest.get_name() + "\" modified file \"" + path.string() + "\"");
}