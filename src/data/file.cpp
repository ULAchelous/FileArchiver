#include<data/file.h>
#include<filesystem>
#include <iostream>
using namespace std;
error_code fa_move_file(const std::filesystem::path& file, const std::filesystem::path& target){
    error_code ec;
    filesystem::create_directories(target.parent_path(),ec);
    if(ec)
        return ec;
    filesystem::rename(file,target,ec);
    return ec;
}