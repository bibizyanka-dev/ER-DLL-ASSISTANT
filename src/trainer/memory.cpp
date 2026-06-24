#include "memory.h"
#include <psapi.h>

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

bool Write(uintptr_t address, const void* buffer, size_t size) {
  if (address == 0) {
    return false;
  }

  __try {
    memcpy(reinterpret_cast<void*>(address), buffer, size);
    return true;
  } __except (EXCEPTION_EXECUTE_HANDLER) {
    return false;
  }
}

}  // namespace Memory
