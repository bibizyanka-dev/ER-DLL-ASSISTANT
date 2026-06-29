#pragma once
#include <string>
#include <cstdarg>

class Logger {
public:
    static void Log(const char* format, ...);
    
    static void Log(const std::string& msg);
};

inline void Log(const char* format, ...) {
    va_list args;
    va_start(args, format);
    Logger::Log(format, args);
    va_end(args);
}

inline void Log(const std::string& msg) {
    Logger::Log(msg);
}

