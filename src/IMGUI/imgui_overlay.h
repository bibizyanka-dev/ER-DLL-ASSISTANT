#pragma once

#include <dxgi1_4.h>

class ImGuiOverlay {
 public:
  static ImGuiOverlay& Instance();

  bool IsInitialized() const { return initialized_; }
  bool IsMenuVisible() const { return showMenu_; }

  void Initialize(IDXGISwapChain3* swapChain);
  void BeginFrame();
  void RenderMenu();
  void RenderDrawData(IDXGISwapChain3* swapChain);
  LRESULT HandleWndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);

 private:
  ImGuiOverlay() = default;

  bool showMenu_ = false;
  bool initialized_ = false;
};
