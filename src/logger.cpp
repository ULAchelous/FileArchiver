#include<iostream>
#include <chrono>
#include <sstream>
#include "logger.h"
using namespace logger;
Logger::Logger(const std::string& name):_name(name){}

std::string Logger::_build_log_msg(LoggerType type,const std::string& message){
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
    return std::string(colors::BLUE) + "["+time_str+"] " + colors::RESET + color_code+"["+type_str+"] "+colors::RESET+colors::CYAN+"("+_name+") "+colors::RESET+ message;
}
void Logger::info(const std::string& message){
    std::cout<<_build_log_msg(LoggerType::INFO, message)<<std::endl;
}
void Logger::error(const std::string& message){
    std::cerr<<_build_log_msg(LoggerType::ERROR, message)<<std::endl;
}
void Logger::error(const std::string& msg,const std::error_code& ec){
    std::cerr<<_build_log_msg(LoggerType::ERROR, msg + " Error code: "+ std::to_string(ec.value()) + " Message: " + ec.message())<<std::endl;
}  
