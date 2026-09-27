#include<iostream>
#include <chrono>
#include <sstream>
#include <fstream>
#include "logger.h"
using namespace logger;
Logger::Logger(const std::string& name):_name(name){}
Logger::Logger(const std::string& name,std::filesystem::path log_file):_name(name),_log_file_path(log_file){}

std::string Logger::_build_log_msg(LoggerType type,const std::string& message,bool is_colored){
    auto now = std::chrono::system_clock::now();
    std::time_t now_time = std::chrono::system_clock::to_time_t(now);
    std::string time_str = (std::stringstream() << std::put_time(std::localtime(&now_time), "%H:%M:%S")).str();
    std::string type_str;
    std::string color_code;
    switch(type){
        case LoggerType::INFO:
            type_str = "INFO";
            color_code = colors::GREEN;
            break;
        case LoggerType::ERROR:
            type_str = "ERROR";
            color_code = colors::RED;
            break;
        default:
            type_str = "UNKNOWN";
            color_code = colors::RESET;
    }
    if(is_colored)
        return std::string(colors::BLUE) + "["+time_str+"] " + colors::RESET + color_code+"["+type_str+"] "+colors::RESET+colors::CYAN+"("+_name+") "+colors::RESET+ message;
    else
        return "["+time_str+"] " + "["+type_str+"] " + "("+_name+") "+ message;
}
void Logger::_write(const std::string& str){
    if(!std::filesystem::exists(_log_file_path.parent_path()) && !_log_file_path.parent_path().empty())
        std::filesystem::create_directory(_log_file_path.parent_path());
    std::ofstream file(_log_file_path,std::ios::app);
    if(!file)
        error("Failed to open file: " + _log_file_path.string());
    else
        file << str << std::endl;
}
void Logger::set_log_file(const std::filesystem::path& file){
    _log_file_path = file;
}
void Logger::info(const std::string& message){
    std::cout<<_build_log_msg(LoggerType::INFO, message, true)<<std::endl;
    if(!_log_file_path.empty())
        _write(_build_log_msg(LoggerType::INFO, message, false));
}
void Logger::error(const std::string& message){
    std::cerr<<_build_log_msg(LoggerType::ERROR, message,true)<<std::endl;
    if(!_log_file_path.empty())
        _write(_build_log_msg(LoggerType::ERROR, message,false));
}
void Logger::error(const std::string& msg,const std::error_code& ec){
    std::string msg1 = msg + " Error code: "+ std::to_string(ec.value()) + " Message: " + ec.message();
    std::cerr<<_build_log_msg(LoggerType::ERROR,msg1,true)<<std::endl;
    if(!_log_file_path.empty())
        _write(_build_log_msg(LoggerType::ERROR,msg1,false));
}  
