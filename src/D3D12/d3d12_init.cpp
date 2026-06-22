#include "d3d12_init.h"

#include <cassert>
#include <cstring>

#include <d3d12.h>
#include <dxgi1_4.h>

#include "MinHook/include/MinHook.h"
#include "d3d12_interface.h"

#pragma comment(lib, "d3d12.lib")

namespace {

WNDCLASSEX g_windowClass{};
HWND g_windowHwnd = nullptr;
uintx_t* g_methodsTable = nullptr;

bool InitWindow() {
  g_windowClass.cbSize = sizeof(WNDCLASSEX);
  g_windowClass.style = CS_HREDRAW | CS_VREDRAW;
  g_windowClass.lpfnWndProc = DefWindowProc;
  g_windowClass.cbClsExtra = 0;
  g_windowClass.cbWndExtra = 0;
  g_windowClass.hInstance = GetModuleHandle(nullptr);
  g_windowClass.hIcon = nullptr;
  g_windowClass.hCursor = nullptr;
  g_windowClass.hbrBackground = nullptr;
  g_windowClass.lpszMenuName = nullptr;
  g_windowClass.lpszClassName = "MJ";
  g_windowClass.hIconSm = nullptr;
  RegisterClassEx(&g_windowClass);
  g_windowHwnd = CreateWindow(g_windowClass.lpszClassName, "DirectX Window", WS_OVERLAPPEDWINDOW, 0, 0, 100, 100, nullptr, nullptr,
                              g_windowClass.hInstance, nullptr);
  return g_windowHwnd != nullptr;
}

bool DeleteWindow() {
  DestroyWindow(g_windowHwnd);
  UnregisterClass(g_windowClass.lpszClassName, g_windowClass.hInstance);
  return g_windowHwnd == nullptr;
}

}  // namespace

namespace D3D12Hook {

bool Init() {
  if (!InitWindow()) {
    return false;
  }

  HMODULE d3d12Module = GetModuleHandle("d3d12.dll");
  HMODULE dxgiModule = GetModuleHandle("dxgi.dll");
  if (d3d12Module == nullptr || dxgiModule == nullptr) {
    DeleteWindow();
    return false;
  }

  void* createDxgiFactory = GetProcAddress(dxgiModule, "CreateDXGIFactory");
  if (createDxgiFactory == nullptr) {
    DeleteWindow();
    return false;
  }

  IDXGIFactory* factory = nullptr;
  if (reinterpret_cast<long(__stdcall*)(const IID&, void**)>(createDxgiFactory)(__uuidof(IDXGIFactory), reinterpret_cast<void**>(&factory)) < 0) {
    DeleteWindow();
    return false;
  }

  IDXGIAdapter* adapter = nullptr;
  if (factory->EnumAdapters(0, &adapter) == DXGI_ERROR_NOT_FOUND) {
    DeleteWindow();
    return false;
  }

  void* d3d12CreateDevice = GetProcAddress(d3d12Module, "D3D12CreateDevice");
  if (d3d12CreateDevice == nullptr) {
    DeleteWindow();
    return false;
  }

  ID3D12Device* device = nullptr;
  if (reinterpret_cast<long(__stdcall*)(IUnknown*, D3D_FEATURE_LEVEL, const IID&, void**)>(d3d12CreateDevice)(
          adapter, D3D_FEATURE_LEVEL_11_0, __uuidof(ID3D12Device), reinterpret_cast<void**>(&device)) < 0) {
    DeleteWindow();
    return false;
  }

  D3D12_COMMAND_QUEUE_DESC queueDesc{};
  queueDesc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;
  queueDesc.Priority = 0;
  queueDesc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
  queueDesc.NodeMask = 0;

  ID3D12CommandQueue* commandQueue = nullptr;
  if (device->CreateCommandQueue(&queueDesc, __uuidof(ID3D12CommandQueue), reinterpret_cast<void**>(&commandQueue)) < 0) {
    DeleteWindow();
    return false;
  }

  ID3D12CommandAllocator* commandAllocator = nullptr;
  if (device->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, __uuidof(ID3D12CommandAllocator),
                                     reinterpret_cast<void**>(&commandAllocator)) < 0) {
    DeleteWindow();
    return false;
  }

  ID3D12GraphicsCommandList* commandList = nullptr;
  if (device->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, commandAllocator, nullptr, __uuidof(ID3D12GraphicsCommandList),
                                reinterpret_cast<void**>(&commandList)) < 0) {
    DeleteWindow();
    return false;
  }

  DXGI_RATIONAL refreshRate{};
  refreshRate.Numerator = 60;
  refreshRate.Denominator = 1;

  DXGI_MODE_DESC bufferDesc{};
  bufferDesc.Width = 100;
  bufferDesc.Height = 100;
  bufferDesc.RefreshRate = refreshRate;
  bufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
  bufferDesc.ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED;
  bufferDesc.Scaling = DXGI_MODE_SCALING_UNSPECIFIED;

  DXGI_SAMPLE_DESC sampleDesc{};
  sampleDesc.Count = 1;
  sampleDesc.Quality = 0;

  DXGI_SWAP_CHAIN_DESC swapChainDesc{};
  swapChainDesc.BufferDesc = bufferDesc;
  swapChainDesc.SampleDesc = sampleDesc;
  swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
  swapChainDesc.BufferCount = 2;
  swapChainDesc.OutputWindow = g_windowHwnd;
  swapChainDesc.Windowed = 1;
  swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
  swapChainDesc.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;

  IDXGISwapChain* swapChain = nullptr;
  if (factory->CreateSwapChain(commandQueue, &swapChainDesc, &swapChain) < 0) {
    DeleteWindow();
    return false;
  }

  g_methodsTable = static_cast<uintx_t*>(calloc(150, sizeof(uintx_t)));
  memcpy(g_methodsTable, *reinterpret_cast<uintx_t**>(device), 44 * sizeof(uintx_t));
  memcpy(g_methodsTable + 44, *reinterpret_cast<uintx_t**>(commandQueue), 19 * sizeof(uintx_t));
  memcpy(g_methodsTable + 44 + 19, *reinterpret_cast<uintx_t**>(commandAllocator), 9 * sizeof(uintx_t));
  memcpy(g_methodsTable + 44 + 19 + 9, *reinterpret_cast<uintx_t**>(commandList), 60 * sizeof(uintx_t));
  memcpy(g_methodsTable + 44 + 19 + 9 + 60, *reinterpret_cast<uintx_t**>(swapChain), 18 * sizeof(uintx_t));

  MH_Initialize();

  device->Release();
  commandQueue->Release();
  commandAllocator->Release();
  commandList->Release();
  swapChain->Release();
  DeleteWindow();
  return true;
}

bool CreateHook(uint16_t index, void** original, void* function) {
  assert(original != nullptr && function != nullptr);
  void* target = reinterpret_cast<void*>(g_methodsTable[index]);
  if (MH_CreateHook(target, function, original) != MH_OK || MH_EnableHook(target) != MH_OK) {
    return false;
  }
  return true;
}

void DisableHook(uint16_t index) {
  MH_DisableHook(reinterpret_cast<void*>(g_methodsTable[index]));
}

void DisableAll() {
  MH_DisableHook(MH_ALL_HOOKS);
  free(g_methodsTable);
  g_methodsTable = nullptr;
}

}  // namespace D3D12Hook
