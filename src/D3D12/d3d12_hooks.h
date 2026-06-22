#pragma once

#include <windows.h>

#include <d3d12.h>
#include <dxgi1_4.h>

HRESULT APIENTRY hkPresent(IDXGISwapChain3* swapChain, UINT syncInterval, UINT flags);
void APIENTRY hkExecuteCommandLists(ID3D12CommandQueue* queue, UINT numCommandLists, ID3D12CommandList* commandLists);
void APIENTRY hkDrawInstanced(ID3D12GraphicsCommandList* commandList, UINT vertexCountPerInstance, UINT instanceCount,
                              UINT startVertexLocation, UINT startInstanceLocation);
