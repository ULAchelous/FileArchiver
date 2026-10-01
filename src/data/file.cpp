#include<data/file.h>
#include<filesystem>
#include <iostream>

#include "registry.h"
#include "app.h"

using namespace std;


error_code fa_move_file(const std::filesystem::path& file, const std::filesystem::path& target){
    error_code ec;
    filesystem::create_directories(target.parent_path(),ec);
    if(ec)
        return ec;
    filesystem::rename(file,target,ec);
    return ec;
}
bool fa_path_contains(const std::filesystem::path base,const std::filesystem::path path){
    auto rel = path.lexically_relative(base);
    return !rel.empty() & *rel.begin() != "..";
}

const FileType* fa_get_file_type(const std::filesystem::path& path,Context& ctx){
    std::string ext = path.extension().string();
    std::vector<FileType*> types;
    try{
        types = ctx.registries.get_registry<FileType>().get_types_by_ext(ext);
    }catch(const std::exception& e){
        ctx.registries.get_registry<FileType>().register_t(ext,FileType(ext,{ext},nullptr));
        types = ctx.registries.get_registry<FileType>().get_types_by_ext(ext);
    }

    const FileType* type = types[0];
    for(const FileType* t : types){
        if(t->classify_func != nullptr && t->classify_func(path.string().c_str())){
            type = t;
            break;
        }
    }

    return type;
}