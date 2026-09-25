#pragma once
#include <string>
#include <system_error>
#include <filesystem>

namespace logger{
    namespace colors {
        constexpr const char* RED    = "\033[31m";
        constexpr const char* YELLOW = "\033[33m";
        constexpr const char* GREEN  = "\033[32m";
        constexpr const char* RESET  = "\033[0m";
        constexpr const char* BLUE   = "\033[34m";
        constexpr const char* CYAN   = "\033[36m";
    }
    enum class LoggerType {
        INFO,
        ERROR
    };

    class Logger{
        public:
            Logger(const std::string& name);
            void info(const std::string& message);
            void error(const std::string& message);
            void error(const std::string& msg,const std::error_code& ec);
        private:
            std::string _name;
            std::filesystem::path _log_file_path;
       std::string _build_log_msg(LoggerType type,const std::string& message);
    };
}
inline logger::Logger LOGGER("FileArchiver");