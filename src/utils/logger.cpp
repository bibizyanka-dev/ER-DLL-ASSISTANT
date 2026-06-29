#include "logger.h"
#include <cstdio>
#include <cstdarg>
#include <windows.h>

void Logger::Log(const char* format, ...)
{
    char buffer[2048] = {0};
    va_list args;
    va_start(args, format);
    vsnprintf(buffer, sizeof(buffer), format, args);
    va_end(args);

    FILE* f = nullptr;
    if (fopen_s(&f, "EldenRingPatch.log", "a") == 0 && f)
    {
        SYSTEMTIME st;
        GetLocalTime(&st);

        fprintf(f, "[%02d:%02d:%02d] %s\n", 
                st.wHour, st.wMinute, st.wSecond, buffer);

        fclose(f);
    }
}

void Logger::Log(const std::string& msg)
{
    Log("%s", msg.c_str());
}
