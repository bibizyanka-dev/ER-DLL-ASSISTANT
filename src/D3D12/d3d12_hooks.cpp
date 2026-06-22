#include "d3d12_hooks.h"

#include "src/d3d12/d3d12_interface.h"
#include "src/d3d12/d3d12_types.h"
#include "src/imgui/imgui_overlay.h"

HRESULT APIENTRY hkPresent(IDXGISwapChain3* swapChain, UINT syncInterval, UINT flags) {
  auto& overlay = ImGuiOverlay::Instance();

  if (!overlay.IsInitialized()) {
    overlay.Initialize(swapChain);
  }

  if (DirectX12Interface::CommandQueue == nullptr) {
    return oPresent(swapChain, syncInterval, flags);
  }

  overlay.BeginFrame();
  overlay.RenderMenu();
  overlay.RenderDrawData(swapChain);

  return oPresent(swapChain, syncInterval, flags);
}

void hkExecuteCommandLists(ID3D12CommandQueue* queue, UINT numCommandLists, ID3D12CommandList* commandLists) {
  if (!DirectX12Interface::CommandQueue) {
    DirectX12Interface::CommandQueue = queue;
  }

  oExecuteCommandLists(queue, numCommandLists, commandLists);
}

void APIENTRY hkDrawInstanced(ID3D12GraphicsCommandList* commandList, UINT vertexCountPerInstance, UINT instanceCount,
                              UINT startVertexLocation, UINT startInstanceLocation) {
  oDrawInstanced(commandList, vertexCountPerInstance, instanceCount, startVertexLocation, startInstanceLocation);
}
