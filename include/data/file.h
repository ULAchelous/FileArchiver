#pragma once
#include<string>
#include<vector>
#include <filesystem>

struct FileType{
    FileType() = default;
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

std::error_code fa_move_file(const std::filesystem::path& file,const std::filesystem::path& target);
