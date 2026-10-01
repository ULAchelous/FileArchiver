#pragma once
#include<string>
#include<vector>
#include <filesystem>
#include "plugin/types.h"

struct Context;

struct FileType{
    FileType(fa_file_t type): id(type.id), classify_func(type.classify_func) {
        for(int i=0;i<type.ext_count;i++)
            exts.push_back(type.exts[i]);
    };
    FileType(const std::string& _id,const std::vector<std::string>& _exts,bool (*_classify_func)(const char* file)):id(_id),exts(_exts),classify_func(_classify_func){}
    std::string id;
    std::vector<std::string> exts;
    bool (*classify_func)(const char* file);
};

struct File{
    File() = default;
    File(const std::filesystem::path& _path,const std::string& _prefix,const std::string& _suffix,const FileType* _type):path(_path),prefix(_prefix),suffix(_suffix),type(_type){}
    std::filesystem::path path;
    std::string prefix;
    std::string suffix;
    const FileType* type = nullptr;
};

namespace repo { struct Repository; }

struct DirNode{
    std::filesystem::path path;
    repo::Repository* repo;
    std::vector<DirNode*> children;
};

const FileType* fa_get_file_type(const std::filesystem::path& path,Context& ctx);
std::error_code fa_move_file(const std::filesystem::path& file,const std::filesystem::path& target);
bool fa_path_contains(const std::filesystem::path base,const std::filesystem::path path);
