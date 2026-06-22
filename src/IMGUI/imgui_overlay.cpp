#include "imgui_overlay.h"

#include <windows.h>

#include "ImGui/imgui.h"
#include "ImGui/imgui_impl_dx12.h"
#include "ImGui/imgui_impl_win32.h"
#include "src/core/process.h"
#include "src/d3d12/d3d12_interface.h"
#include "src/trainer/player_trainer.h"

extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

LRESULT CALLBACK OverlayWndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
  return ImGuiOverlay::Instance().HandleWndProc(hwnd, msg, wParam, lParam);
}

ImGuiOverlay& ImGuiOverlay::Instance() {
  static ImGuiOverlay instance;
  return instance;
}

LRESULT ImGuiOverlay::HandleWndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
  if (showMenu_) {
    ImGui_ImplWin32_WndProcHandler(hwnd, msg, wParam, lParam);
    return true;
  }
  return CallWindowProc(Process::WndProc, hwnd, msg, wParam, lParam);
}

void ImGuiOverlay::Initialize(IDXGISwapChain3* swapChain) {
  if (initialized_) {
    return;
  }

  if (FAILED(swapChain->GetDevice(__uuidof(ID3D12Device), reinterpret_cast<void**>(&DirectX12Interface::Device)))) {
    return;
  }

  ImGui::CreateContext();

  ImGuiIO& io = ImGui::GetIO();
  io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;

  DXGI_SWAP_CHAIN_DESC desc{};
  swapChain->GetDesc(&desc);
  desc.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;
  desc.OutputWindow = Process::Hwnd;
  desc.Windowed = (GetWindowLongPtr(Process::Hwnd, GWL_STYLE) & WS_POPUP) == 0;

  DirectX12Interface::BuffersCounts = desc.BufferCount;
  DirectX12Interface::FrameContextArray = new DirectX12Interface::FrameContext[DirectX12Interface::BuffersCounts];

  D3D12_DESCRIPTOR_HEAP_DESC descriptorImGuiRender{};
  descriptorImGuiRender.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
  descriptorImGuiRender.NumDescriptors = DirectX12Interface::BuffersCounts;
  descriptorImGuiRender.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;

  if (DirectX12Interface::Device->CreateDescriptorHeap(&descriptorImGuiRender, IID_PPV_ARGS(&DirectX12Interface::DescriptorHeapImGuiRender)) !=
      S_OK) {
    return;
  }

  ID3D12CommandAllocator* allocator = nullptr;
  if (DirectX12Interface::Device->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(&allocator)) != S_OK) {
    return;
  }

  for (size_t i = 0; i < DirectX12Interface::BuffersCounts; i++) {
    DirectX12Interface::FrameContextArray[i].CommandAllocator = allocator;
  }

  if (DirectX12Interface::Device->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, allocator, nullptr,
                                                    IID_PPV_ARGS(&DirectX12Interface::CommandList)) != S_OK ||
      DirectX12Interface::CommandList->Close() != S_OK) {
    return;
  }

  D3D12_DESCRIPTOR_HEAP_DESC descriptorBackBuffers{};
  descriptorBackBuffers.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
  descriptorBackBuffers.NumDescriptors = DirectX12Interface::BuffersCounts;
  descriptorBackBuffers.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
  descriptorBackBuffers.NodeMask = 1;

  if (DirectX12Interface::Device->CreateDescriptorHeap(&descriptorBackBuffers, IID_PPV_ARGS(&DirectX12Interface::DescriptorHeapBackBuffers)) !=
      S_OK) {
    return;
  }

  const auto rtvDescriptorSize = DirectX12Interface::Device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
  D3D12_CPU_DESCRIPTOR_HANDLE rtvHandle = DirectX12Interface::DescriptorHeapBackBuffers->GetCPUDescriptorHandleForHeapStart();

  for (size_t i = 0; i < DirectX12Interface::BuffersCounts; i++) {
    ID3D12Resource* backBuffer = nullptr;
    DirectX12Interface::FrameContextArray[i].DescriptorHandle = rtvHandle;
    swapChain->GetBuffer(static_cast<UINT>(i), IID_PPV_ARGS(&backBuffer));
    DirectX12Interface::Device->CreateRenderTargetView(backBuffer, nullptr, rtvHandle);
    DirectX12Interface::FrameContextArray[i].Resource = backBuffer;
    rtvHandle.ptr += rtvDescriptorSize;
  }

  ImGui_ImplWin32_Init(Process::Hwnd);
  ImGui_ImplDX12_Init(DirectX12Interface::Device, static_cast<int>(DirectX12Interface::BuffersCounts), DXGI_FORMAT_R8G8B8A8_UNORM,
                      DirectX12Interface::DescriptorHeapImGuiRender,
                      DirectX12Interface::DescriptorHeapImGuiRender->GetCPUDescriptorHandleForHeapStart(),
                      DirectX12Interface::DescriptorHeapImGuiRender->GetGPUDescriptorHandleForHeapStart());
  ImGui_ImplDX12_CreateDeviceObjects();
  ImGui::GetIO().ImeWindowHandle = Process::Hwnd;
  Process::WndProc = reinterpret_cast<WNDPROC>(SetWindowLongPtr(Process::Hwnd, GWLP_WNDPROC, reinterpret_cast<LONG_PTR>(OverlayWndProc)));

  initialized_ = true;
}

void ImGuiOverlay::BeginFrame() {
  if (GetAsyncKeyState(VK_HOME) & 1) {
    showMenu_ = !showMenu_;
  }

  ImGui_ImplDX12_NewFrame();
  ImGui_ImplWin32_NewFrame();
  ImGui::NewFrame();
  ImGui::GetIO().MouseDrawCursor = showMenu_;
}

void ImGuiOverlay::RenderMenu() {
  if (!showMenu_) {
    return;
  }

  if (ImGui::Begin("Elden Ring Mod", &showMenu_, ImGuiWindowFlags_AlwaysAutoResize)) {
    if (ImGui::BeginTabBar("TabBar")) {
      if (ImGui::BeginTabItem("Player")) {
        PlayerTrainer::Instance().RenderTab();
        ImGui::EndTabItem();
      }
      ImGui::EndTabBar();
    }
  }
  ImGui::End();
}

void ImGuiOverlay::RenderDrawData(IDXGISwapChain3* swapChain) {
  ImGui::EndFrame();

  DirectX12Interface::FrameContext& currentFrameContext =
      DirectX12Interface::FrameContextArray[swapChain->GetCurrentBackBufferIndex()];
  currentFrameContext.CommandAllocator->Reset();

  D3D12_RESOURCE_BARRIER barrier{};
  barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
  barrier.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
  barrier.Transition.pResource = currentFrameContext.Resource;
  barrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
  barrier.Transition.StateBefore = D3D12_RESOURCE_STATE_PRESENT;
  barrier.Transition.StateAfter = D3D12_RESOURCE_STATE_RENDER_TARGET;

  DirectX12Interface::CommandList->Reset(currentFrameContext.CommandAllocator, nullptr);
  DirectX12Interface::CommandList->ResourceBarrier(1, &barrier);
  DirectX12Interface::CommandList->OMSetRenderTargets(1, &currentFrameContext.DescriptorHandle, FALSE, nullptr);
  DirectX12Interface::CommandList->SetDescriptorHeaps(1, &DirectX12Interface::DescriptorHeapImGuiRender);

  ImGui::Render();
  ImGui_ImplDX12_RenderDrawData(ImGui::GetDrawData(), DirectX12Interface::CommandList);

  barrier.Transition.StateBefore = D3D12_RESOURCE_STATE_RENDER_TARGET;
  barrier.Transition.StateAfter = D3D12_RESOURCE_STATE_PRESENT;
  DirectX12Interface::CommandList->ResourceBarrier(1, &barrier);
  DirectX12Interface::CommandList->Close();
  DirectX12Interface::CommandQueue->ExecuteCommandLists(
      1, reinterpret_cast<ID3D12CommandList* const*>(&DirectX12Interface::CommandList));
}
