#pragma once

#include <cstddef>
#include <cstdint>

namespace Memory {

bool Read(uintptr_t address, void* buffer, size_t size);

template <size_t N>
uintptr_t ResolvePointerChain(uintptr_t base, const uintptr_t (&offsets)[N]) {
  if (!base || N == 0) {
    return 0;
  }

  uintptr_t addr = 0;
  if (!Read(base, &addr, sizeof(addr))) {
    return 0;
  }

  for (size_t i = 0; i < N - 1; i++) {
    if (!addr) {
      return 0;
    }

    uintptr_t nextAddr = 0;
    if (!Read(addr + offsets[i], &nextAddr, sizeof(nextAddr))) {
      return 0;
    }

    addr = nextAddr;
  }

  return addr ? addr + offsets[N - 1] : 0;
}

}  // namespace Memory
