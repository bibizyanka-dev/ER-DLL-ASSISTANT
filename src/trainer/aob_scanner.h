#pragma once

#include <cstddef>
#include <cstdint>
#include <string>
#include <unordered_map>
#include <vector>
#include <windows.h>

#include "player_data.h"

extern std::vector<AOBEntry> aobList;

uintptr_t FindPattern(HMODULE module, const char* pattern);
std::unordered_map<std::string, uintptr_t> ScanAllAOBs(HMODULE module = nullptr);

class AOBRegistry {
 public:
  bool EnsureScanned(HMODULE module);
  void ApplyTo(GameAOBAddresses& out) const;
  uintptr_t Get(const char* name) const;

 private:
  std::unordered_map<std::string, uintptr_t> cache_;
  bool scanned_ = false;
};

AOBRegistry& GetAOBRegistry();
