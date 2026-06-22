#include "memory.h"

#include <cstring>

#include <windows.h>

namespace Memory {

bool Read(uintptr_t address, void* buffer, size_t size) {
  if (address == 0) {
    return false;
  }

  __try {
    memcpy(buffer, reinterpret_cast<const void*>(address), size);
    return true;
  } __except (EXCEPTION_EXECUTE_HANDLER) {
    return false;
  }
}

}  // namespace Memory
