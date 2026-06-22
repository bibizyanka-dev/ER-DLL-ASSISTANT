#pragma once

#include <cstdint>

#include <d3d12.h>

#if defined _M_X64
using uintx_t = uint64_t;
#elif defined _M_IX86
using uintx_t = uint32_t;
#endif

namespace DirectX12Interface {

struct FrameContext {
  ID3D12CommandAllocator* CommandAllocator = nullptr;
  ID3D12Resource* Resource = nullptr;
  D3D12_CPU_DESCRIPTOR_HANDLE DescriptorHandle{};
};

inline ID3D12Device* Device = nullptr;
inline ID3D12DescriptorHeap* DescriptorHeapBackBuffers = nullptr;
inline ID3D12DescriptorHeap* DescriptorHeapImGuiRender = nullptr;
inline ID3D12GraphicsCommandList* CommandList = nullptr;
inline ID3D12CommandQueue* CommandQueue = nullptr;
inline uintx_t BuffersCounts = static_cast<uintx_t>(-1);
inline FrameContext* FrameContextArray = nullptr;

}  // namespace DirectX12Interface
