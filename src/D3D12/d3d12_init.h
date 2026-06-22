#pragma once

#include <cstdint>

namespace D3D12Hook {

bool Init();
bool CreateHook(uint16_t index, void** original, void* function);
void DisableHook(uint16_t index);
void DisableAll();

}  // namespace D3D12Hook
