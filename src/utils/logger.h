#pragma once

#include <string>

class Logger {
 public:
  void Log(const std::string& msg);
};

inline void Log(const std::string& msg) {
  static Logger logger;
  logger.Log(msg);
}
