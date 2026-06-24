#include "aob_scanner.h"
#include "memory.h"
#include "../utils/logger.h"
#include <cstdint>
#include <cstdio>
#include <psapi.h>
#include <sstream>
#include <string>

std::vector<AOBEntry> aobList = {
    {"WorldChrMan", "48 8B 05 ?? ?? ?? ?? 48 85 C0 74 0F 48 39 88", 3, 7},
    {"GameDataMan", "48 8B 05 ?? ?? ?? ?? 48 85 C0 74 05 48 8B 40 58 C3 C3", 3, 7},
};

namespace {

bool PatternMatches(uintptr_t addr, const std::vector<uint8_t>& bytes, const std::vector<bool>& mask) {
  std::vector<uint8_t> buffer(bytes.size());
  if (!Memory::Read(addr, buffer.data(), buffer.size())) {
    return false;
  }

  for (size_t i = 0; i < bytes.size(); ++i) {
    if (mask[i] && buffer[i] != bytes[i]) {
      return false;
    }
  }

  return true;
}

bool IsExecutableRegion(const MEMORY_BASIC_INFORMATION& mbi) {
  if (mbi.State != MEM_COMMIT) {
    return false;
  }

  if (mbi.Protect & (PAGE_GUARD | PAGE_NOACCESS)) {
    return false;
  }

  return (mbi.Protect & (PAGE_EXECUTE | PAGE_EXECUTE_READ | PAGE_EXECUTE_READWRITE | PAGE_EXECUTE_WRITECOPY)) != 0;
}

bool ParsePattern(const char* pattern, std::vector<uint8_t>& bytes, std::vector<bool>& mask) {
  std::stringstream ss(pattern);
  std::string token;

  while (ss >> token) {
    if (token == "?" || token == "??") {
      bytes.push_back(0);
      mask.push_back(false);
    } else {
      bytes.push_back(static_cast<uint8_t>(std::stoul(token, nullptr, 16)));
      mask.push_back(true);
    }
  }

  return !bytes.empty();
}

uintptr_t ResolveAOBAddress(uintptr_t found, const AOBEntry& entry) {
  if (entry.additional > 0) {
    int32_t rel = 0;
    if (!Memory::Read(found + entry.offset, &rel, sizeof(rel))) {
      return 0;
    }
    return found + entry.additional + rel;
  }

  return found + entry.offset;
}

}  // namespace

uintptr_t FindPattern(HMODULE module, const char* pattern) {
  if (!module) {
    module = GetModuleHandle(nullptr);
  }

  MODULEINFO mi{};
  if (!K32GetModuleInformation(GetCurrentProcess(), module, &mi, sizeof(mi))) {
    Log("[-] K32GetModuleInformation failed");
    return 0;
  }

  std::vector<uint8_t> bytes;
  std::vector<bool> mask;
  if (!ParsePattern(pattern, bytes, mask)) {
    return 0;
  }

  const uintptr_t moduleStart = reinterpret_cast<uintptr_t>(mi.lpBaseOfDll);
  const uintptr_t moduleEnd = moduleStart + mi.SizeOfImage;

  for (uintptr_t region = moduleStart; region < moduleEnd;) {
    MEMORY_BASIC_INFORMATION mbi{};
    if (!VirtualQuery(reinterpret_cast<LPCVOID>(region), &mbi, sizeof(mbi))) {
      break;
    }

    const uintptr_t regionStart = reinterpret_cast<uintptr_t>(mbi.BaseAddress);
    const uintptr_t regionEnd = regionStart + mbi.RegionSize;

    if (IsExecutableRegion(mbi)) {
      const uintptr_t scanStart = (regionStart < moduleStart) ? moduleStart : regionStart;
      const uintptr_t scanEnd = (regionEnd > moduleEnd) ? moduleEnd : regionEnd;

      if (scanEnd > scanStart + bytes.size()) {
        for (uintptr_t addr = scanStart; addr < scanEnd - bytes.size() + 1; ++addr) {
          if (PatternMatches(addr, bytes, mask)) {
            return addr;
          }
        }
      }
    }

    region = regionEnd;
  }

  return 0;
}

std::unordered_map<std::string, uintptr_t> ScanAllAOBs(HMODULE module) {
  std::unordered_map<std::string, uintptr_t> results;

  for (const auto& entry : aobList) {
    const uintptr_t found = FindPattern(module, entry.pattern.c_str());
    if (!found) {
      Log("[-] " + entry.name + " not found");
      continue;
    }

    const uintptr_t addr = ResolveAOBAddress(found, entry);
    if (!addr) {
      Log("[-] " + entry.name + " resolve failed");
      continue;
    }

    results[entry.name] = addr;

    char msg[128];
    sprintf_s(msg, "[+] %s = 0x%llX", entry.name.c_str(), static_cast<unsigned long long>(addr));
    Log(msg);
  }

  return results;
}

namespace {

struct AOBFieldBinding {
  const char* name;
  size_t offset;
};

constexpr AOBFieldBinding kAOBFieldBindings[] = {
    {"WorldChrMan", offsetof(GameAOBAddresses, WorldChrMan)},
    {"GameDataMan", offsetof(GameAOBAddresses, GameDataMan)},
};

}  // namespace

AOBRegistry& GetAOBRegistry() {
  static AOBRegistry registry;
  return registry;
}

bool AOBRegistry::EnsureScanned(HMODULE module) {
  if (scanned_) {
    return !cache_.empty();
  }

  cache_ = ScanAllAOBs(module);
  scanned_ = true;
  return !cache_.empty();
}

void AOBRegistry::ApplyTo(GameAOBAddresses& out) const {
  for (const auto& binding : kAOBFieldBindings) {
    const auto it = cache_.find(binding.name);
    if (it == cache_.end()) {
      continue;
    }

    *reinterpret_cast<uintptr_t*>(reinterpret_cast<char*>(&out) + binding.offset) = it->second;
  }
}

uintptr_t AOBRegistry::Get(const char* name) const {
  const auto it = cache_.find(name);
  return it != cache_.end() ? it->second : 0;
}
