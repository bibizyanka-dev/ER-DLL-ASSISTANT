#include <windows.h>
#include <psapi.h>

#include "src/core/process.h"
#include "src/D3D12/d3d12_hooks.h"
#include "src/D3D12/d3d12_init.h"
#include "src/D3D12/d3d12_types.h"
#include "src/trainer/player_trainer.h"
#include "src/utils/logger.h"

DWORD WINAPI MainThread(LPVOID lpParameter) {
  bool windowFocus = false;
  while (!windowFocus) {
    DWORD foregroundWindowProcessId = 0;
    GetWindowThreadProcessId(GetForegroundWindow(), &foregroundWindowProcessId);
    if (GetCurrentProcessId() == foregroundWindowProcessId) {
      Process::ID = GetCurrentProcessId();
      Process::Handle = GetCurrentProcess();
      Process::Hwnd = GetForegroundWindow();

      RECT tempRect{};
      GetWindowRect(Process::Hwnd, &tempRect);
      Process::WindowWidth = tempRect.right - tempRect.left;
      Process::WindowHeight = tempRect.bottom - tempRect.top;

      static char tempTitle[MAX_PATH];
      GetWindowText(Process::Hwnd, tempTitle, sizeof(tempTitle));
      Process::Title = tempTitle;

      static char tempClassName[MAX_PATH];
      GetClassName(Process::Hwnd, tempClassName, sizeof(tempClassName));
      Process::ClassName = tempClassName;

      static char tempPath[MAX_PATH];
      GetModuleFileNameEx(Process::Handle, nullptr, tempPath, sizeof(tempPath));
      Process::Path = tempPath;

      windowFocus = true;
    }
  }

  bool initHook = false;
  while (!initHook) {
    if (D3D12Hook::Init()) {
      D3D12Hook::CreateHook(54, reinterpret_cast<void**>(&oExecuteCommandLists), reinterpret_cast<void*>(hkExecuteCommandLists));
      D3D12Hook::CreateHook(140, reinterpret_cast<void**>(&oPresent), reinterpret_cast<void*>(hkPresent));
      D3D12Hook::CreateHook(84, reinterpret_cast<void**>(&oDrawInstanced), reinterpret_cast<void*>(hkDrawInstanced));
      initHook = true;
    }
  }

  Sleep(30000);
  HMODULE moduleBase = GetModuleHandleA("eldenring.exe");
  while (true) {
    if (Process::Handle) {
      PlayerTrainer::Instance().MemoryUpdate(moduleBase);
      PlayerTrainer::Instance().PlayerUpdate();
    }
    Sleep(100);
  }

  return 0;
}

BOOL APIENTRY DllMain(HMODULE hModule, DWORD dwReason, LPVOID lpReserved) {
  switch (dwReason) {
    case DLL_PROCESS_ATTACH:
      DisableThreadLibraryCalls(hModule);
      Process::Module = hModule;
      Log("DLL STARTED");
      CreateThread(nullptr, 0, MainThread, nullptr, 0, nullptr);
      break;
    case DLL_PROCESS_DETACH:
      D3D12Hook::DisableAll();
      FreeLibraryAndExitThread(hModule, TRUE);
      break;
    default:
      break;
  }
  return TRUE;
}
