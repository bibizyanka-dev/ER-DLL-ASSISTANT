#pragma once

#include <windows.h>

#include <d3d12.h>
#include <dxgi1_4.h>

using Present12 = HRESULT(APIENTRY*)(IDXGISwapChain* pSwapChain, UINT SyncInterval, UINT Flags);
using DrawInstancedFn = void(APIENTRY*)(ID3D12GraphicsCommandList* dCommandList, UINT VertexCountPerInstance, UINT InstanceCount,
                                      UINT StartVertexLocation, UINT StartInstanceLocation);
using DrawIndexedInstancedFn = void(APIENTRY*)(ID3D12GraphicsCommandList* dCommandList, UINT IndexCountPerInstance, UINT InstanceCount,
                                               UINT StartIndexLocation, INT BaseVertexLocation, UINT StartInstanceLocation);
using ExecuteCommandListsFn = void(APIENTRY*)(ID3D12CommandQueue* queue, UINT NumCommandLists, ID3D12CommandList* ppCommandLists);

inline Present12 oPresent = nullptr;
inline DrawInstancedFn oDrawInstanced = nullptr;
inline DrawIndexedInstancedFn oDrawIndexedInstanced = nullptr;
inline ExecuteCommandListsFn oExecuteCommandLists = nullptr;
