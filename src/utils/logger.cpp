#include "logger.h"

#include <cstdio>
#include <string>

#include <windows.h>

void Logger::Log(const std::string& msg) {
  FILE* f = nullptr;
  if (fopen_s(&f, "EldenRingPatch.log", "a") == 0 && f) {
    SYSTEMTIME st;
    GetLocalTime(&st);
    fprintf(f, "[%02d:%02d:%02d] %s\n", st.wHour, st.wMinute, st.wSecond, msg.c_str());
    fclose(f);
  }
}

