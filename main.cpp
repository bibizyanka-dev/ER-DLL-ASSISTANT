#include "main.h"

#include <minwindef.h>
#include <winnls.h>
#include <winnt.h>
#include <winuser.h>

#include <cstdint>
#include <thread>

#include "ImGui/imgui.h"

int countnum = -1;
bool nopants_enabled = true;

//=========================================================================================================================//

typedef HRESULT(APIENTRY* Present12)(IDXGISwapChain* pSwapChain, UINT SyncInterval, UINT Flags);
Present12 oPresent = NULL;

typedef void(APIENTRY* DrawInstanced)(ID3D12GraphicsCommandList* dCommandList, UINT VertexCountPerInstance, UINT InstanceCount,
                                      UINT StartVertexLocation, UINT StartInstanceLocation);
DrawInstanced oDrawInstanced = NULL;

typedef void(APIENTRY* DrawIndexedInstanced)(ID3D12GraphicsCommandList* dCommandList, UINT IndexCountPerInstance, UINT InstanceCount,
                                             UINT StartIndexLocation, INT BaseVertexLocation, UINT StartInstanceLocation);
DrawIndexedInstanced oDrawIndexedInstanced = NULL;

typedef void(APIENTRY* ExecuteCommandLists)(ID3D12CommandQueue* queue, UINT NumCommandLists, ID3D12CommandList* ppCommandLists);
ExecuteCommandLists oExecuteCommandLists = NULL;

//=========================================================================================================================//

bool ShowMenu = false;
bool ImGui_Initialised = false;

namespace Process {
DWORD ID;
HANDLE Handle;
HWND Hwnd;
HMODULE Module;
WNDPROC WndProc;
int WindowWidth;
int WindowHeight;
LPCSTR Title;
LPCSTR ClassName;
LPCSTR Path;
}  // namespace Process

namespace DirectX12Interface {
ID3D12Device* Device = nullptr;
ID3D12DescriptorHeap* DescriptorHeapBackBuffers;
ID3D12DescriptorHeap* DescriptorHeapImGuiRender;
ID3D12GraphicsCommandList* CommandList;
ID3D12CommandQueue* CommandQueue;

struct _FrameContext {
  ID3D12CommandAllocator* CommandAllocator;
  ID3D12Resource* Resource;
  D3D12_CPU_DESCRIPTOR_HANDLE DescriptorHandle;
};

uintx_t BuffersCounts = -1;
_FrameContext* FrameContext;
}  // namespace DirectX12Interface

uintptr_t basePlayerAttributeOffset = 0x03D5DF38;
uintptr_t baseWorldChrManOffset = 0x03D65F88;
uintptr_t CSChrDataModuleOffsets[4] = {0x10EF8, 0x0, 0x190, 0x0};

uintptr_t healthOffsets[1] = {0x138};
uintptr_t maxHealthOffests[1] = {0x144};
uintptr_t manaOffsets[1] = {0x148};
uintptr_t maxManaOffsets[1] = {0x14C};
uintptr_t staminaOffests[1] = {0x154};
uintptr_t maxStaminaOffsets[1] = {0x15C};

uintptr_t lvlOffsets[2] = {0x8, 0x68};
uintptr_t runesOffsets[2] = {0x8, 0x6C};
uintptr_t vigorOffsets[2] = {0x8, 0x3C};
uintptr_t mindOffsets[2] = {0x8, 0x40};
uintptr_t enduranceOffsets[2] = {0x8, 0x44};
uintptr_t strangthOffsets[2] = {0x8, 0x48};
uintptr_t dexterityOffsets[2] = {0x8, 0x4C};
uintptr_t intelligenceOffsets[2] = {0x8, 0x50};
uintptr_t faithOffsets[2] = {0x8, 0x54};
uintptr_t arcaneOffsets[2] = {0x8, 0x58};

int runesToAdd = 0;

struct RuntimeAddresses {
  uintptr_t hModule;
  uintptr_t lvl;
  uintptr_t health;
  uintptr_t maxHealth;
  uintptr_t mana;
  uintptr_t maxMana;
  uintptr_t stamina;
  uintptr_t maxStamina;
  uintptr_t runes;
  uintptr_t vigor;
  uintptr_t mind;
  uintptr_t endurance;
  uintptr_t strangth;
  uintptr_t dexterity;
  uintptr_t intelligence;
  uintptr_t faith;
  uintptr_t arcane;
};

inline RuntimeAddresses g_addr;

struct PlayerAttributes {
  int vigor = 0;
  int mind = 0;
  int endurance = 0;
  int strangth = 0;
  int dexterity = 0;
  int intelligence = 0;
  int faith = 0;
  int arcane = 0;
};

struct Player {
  int lvl = 0;
  int health = 0;
  int maxHealth = 0;
  int mana = 0;
  int maxMana = 0;
  int stamina = 0;
  int maxStamina = 0;
  int runes = 0;
  bool godMode = false;
  PlayerAttributes attributes;
};

Player g_player;
//=========================================================================================================================//

extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
LRESULT APIENTRY WndProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
  if (ShowMenu) {
    ImGui_ImplWin32_WndProcHandler(hwnd, uMsg, wParam, lParam);
    return true;
  }
  return CallWindowProc(Process::WndProc, hwnd, uMsg, wParam, lParam);
}

//=========================================================================================================================//

void Log(const std::string& msg) {
  FILE* f = nullptr;
  if (fopen_s(&f, "EldenRingPatch.log", "a") == 0 && f) {
    SYSTEMTIME st;
    GetLocalTime(&st);
    fprintf(f, "[%02d:%02d:%02d] %s\n", st.wHour, st.wMinute, st.wSecond, msg.c_str());
    fclose(f);
  }
}

bool ReadMemory(uintptr_t address, void* buffer, size_t size) {
  if (address == 0) return false;

  __try {
    memcpy(buffer, (const void*)address, size);
    return true;
  } __except (EXCEPTION_EXECUTE_HANDLER) {
    return false;
  }
}

template <size_t N>
uintptr_t ResolvePointerChain(uintptr_t base, const uintptr_t (&offsets)[N]) {
  if (!base || N == 0) return 0;

  uintptr_t addr = 0;

  if (!ReadMemory(base, &addr, sizeof(addr))) return 0;

  for (size_t i = 0; i < N - 1; i++) {
    if (!addr) return 0;

    uintptr_t nextAddr = 0;

    if (!ReadMemory(addr + offsets[i], &nextAddr, sizeof(nextAddr))) return 0;

    addr = nextAddr;
  }

  return addr ? addr + offsets[N - 1] : 0;
}

void handlePlayerData(Player* g_player, HMODULE moduleBase) {
  if (!moduleBase) {
    Log("[-] eldenring.exe module not found");
    return;
  }

  uintptr_t baseAttributeAddr = (uintptr_t)moduleBase + basePlayerAttributeOffset;
  uintptr_t baseWorldChrManAddr = (uintptr_t)moduleBase + baseWorldChrManOffset;
  uintptr_t CSChrDataModuleAddr = ResolvePointerChain(baseWorldChrManAddr, CSChrDataModuleOffsets);

  uintptr_t playerInsAddr = 0;
  if (!ReadMemory(CSChrDataModuleAddr, &playerInsAddr, sizeof(uintptr_t))) {
    return;
  }

  if (playerInsAddr == 0 || playerInsAddr == 0xFFFFFFFFFFFFFFFFULL) {
    return;
  }

  g_addr.hModule = (uintptr_t)moduleBase;
  g_addr.lvl = ResolvePointerChain(baseAttributeAddr, lvlOffsets);
  g_addr.health = ResolvePointerChain(CSChrDataModuleAddr, healthOffsets);
  g_addr.maxHealth = ResolvePointerChain(CSChrDataModuleAddr, maxHealthOffests);
  g_addr.mana = ResolvePointerChain(CSChrDataModuleAddr, manaOffsets);
  g_addr.maxMana = ResolvePointerChain(CSChrDataModuleAddr, maxManaOffsets);
  g_addr.stamina = ResolvePointerChain(CSChrDataModuleAddr, staminaOffests);
  g_addr.maxStamina = ResolvePointerChain(CSChrDataModuleAddr, maxStaminaOffsets);
  g_addr.runes = ResolvePointerChain(baseAttributeAddr, runesOffsets);

  g_addr.vigor = ResolvePointerChain(baseAttributeAddr, vigorOffsets);
  g_addr.mind = ResolvePointerChain(baseAttributeAddr, mindOffsets);
  g_addr.endurance = ResolvePointerChain(baseAttributeAddr, enduranceOffsets);
  g_addr.strangth = ResolvePointerChain(baseAttributeAddr, strangthOffsets);
  g_addr.dexterity = ResolvePointerChain(baseAttributeAddr, dexterityOffsets);
  g_addr.intelligence = ResolvePointerChain(baseAttributeAddr, intelligenceOffsets);
  g_addr.faith = ResolvePointerChain(baseAttributeAddr, faithOffsets);
  g_addr.arcane = ResolvePointerChain(baseAttributeAddr, arcaneOffsets);

  g_player->lvl = *(int*)g_addr.lvl;
  g_player->health = *(int*)g_addr.health;
  g_player->maxHealth = *(int*)g_addr.maxHealth;
  g_player->mana = *(int*)g_addr.mana;
  g_player->maxMana = *(int*)g_addr.maxMana;
  g_player->stamina = *(int*)g_addr.stamina;
  g_player->maxStamina = *(int*)g_addr.maxStamina;
  g_player->runes = *(int*)g_addr.runes;

  g_player->attributes.vigor = *(int*)g_addr.vigor;
  g_player->attributes.mind = *(int*)g_addr.mind;
  g_player->attributes.endurance = *(int*)g_addr.endurance;
  g_player->attributes.strangth = *(int*)g_addr.strangth;
  g_player->attributes.dexterity = *(int*)g_addr.dexterity;
  g_player->attributes.intelligence = *(int*)g_addr.intelligence;
  g_player->attributes.faith = *(int*)g_addr.faith;
  g_player->attributes.arcane = *(int*)g_addr.arcane;
}

std::thread godModeThread;
std::atomic_bool godModeRunning{false};

void enableGodMode() {
  while (godModeRunning) {
    *(int*)g_addr.health = g_player.maxHealth;
    *(int*)g_addr.mana = g_player.maxMana;
    *(int*)g_addr.stamina = g_player.maxStamina;

    Sleep(100);
  }
}
void changeAttributeValue(char symbol, uintptr_t attrAddress) {
  if (!attrAddress) return;

  if (symbol == '+') {
    *reinterpret_cast<int*>(attrAddress) += 1;
    *reinterpret_cast<int*>(g_addr.lvl) += 1;
  } else if (symbol == '-') {
    *reinterpret_cast<int*>(attrAddress) -= 1;
    *reinterpret_cast<int*>(g_addr.lvl) -= 1;
  }
}

void RenderPlayerTab() {
  ImGui::Text("Lvl: %d", g_player.lvl);
  ImGui::Text("Health: %d/%d", g_player.health, g_player.maxHealth);
  ImGui::Text("Mana: %d/%d", g_player.mana, g_player.maxMana);
  ImGui::Text("Stamina: %d/%d", g_player.stamina, g_player.maxStamina);
  ImGui::Text("Runes: %d", g_player.runes);

  ImGui::Separator();

  if (ImGui::Checkbox("God Mode", &g_player.godMode)) {
    if (g_player.godMode) {
      godModeRunning = true;
      godModeThread = std::thread(enableGodMode);
    } else {
      godModeRunning = false;

      if (godModeThread.joinable()) godModeThread.join();
    }
  }

  ImGui::InputInt("Add Runes", &runesToAdd);

  if (ImGui::Button("Add")) {
    *(int*)g_addr.runes += runesToAdd;
  }

  ImGui::SameLine();

  if (ImGui::Button("Remove")) {
    *(int*)g_addr.runes -= runesToAdd;
  }

  ImGui::Separator();

  if (ImGui::BeginTable("AttributesTable", 3, ImGuiTableFlags_BordersInnerV | ImGuiTableFlags_RowBg)) {
    ImGui::TableSetupColumn("Name");
    ImGui::TableSetupColumn("Value");
    ImGui::TableSetupColumn("Actions");

    auto Row = [&](const char* name, int& value, uintptr_t addr) {
      ImGui::TableNextRow();

      // NAME
      ImGui::TableNextColumn();
      ImGui::TextUnformatted(name);

      // VALUE
      ImGui::TableNextColumn();
      ImGui::Text("%d", value);

      // ACTIONS
      ImGui::TableNextColumn();

      ImGui::PushID(name);

      if (ImGui::SmallButton("+")) {
        changeAttributeValue('+', addr);
      }

      ImGui::SameLine();

      if (ImGui::SmallButton("-")) {
        changeAttributeValue('-', addr);
      }

      ImGui::PopID();
    };

    Row("Vigor", g_player.attributes.vigor, g_addr.vigor);
    Row("Mind", g_player.attributes.mind, g_addr.mind);
    Row("Endurance", g_player.attributes.endurance, g_addr.endurance);
    Row("Strength", g_player.attributes.strangth, g_addr.strangth);
    Row("Dexterity", g_player.attributes.dexterity, g_addr.dexterity);
    Row("Intelligence", g_player.attributes.intelligence, g_addr.intelligence);
    Row("Faith", g_player.attributes.faith, g_addr.faith);
    Row("Arcane", g_player.attributes.arcane, g_addr.arcane);

    ImGui::EndTable();
  }
}

HRESULT APIENTRY hkPresent(IDXGISwapChain3* pSwapChain, UINT SyncInterval, UINT Flags) {
  if (!ImGui_Initialised) {
    if (SUCCEEDED(pSwapChain->GetDevice(__uuidof(ID3D12Device), (void**)&DirectX12Interface::Device))) {
      ImGui::CreateContext();

      ImGuiIO& io = ImGui::GetIO();
      (void)io;
      io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;

      DXGI_SWAP_CHAIN_DESC Desc;
      pSwapChain->GetDesc(&Desc);
      Desc.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;
      Desc.OutputWindow = Process::Hwnd;
      Desc.Windowed = ((GetWindowLongPtr(Process::Hwnd, GWL_STYLE) & WS_POPUP) != 0) ? false : true;

      DirectX12Interface::BuffersCounts = Desc.BufferCount;
      DirectX12Interface::FrameContext = new DirectX12Interface::_FrameContext[DirectX12Interface::BuffersCounts];

      D3D12_DESCRIPTOR_HEAP_DESC DescriptorImGuiRender = {};
      DescriptorImGuiRender.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
      DescriptorImGuiRender.NumDescriptors = DirectX12Interface::BuffersCounts;
      DescriptorImGuiRender.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;

      if (DirectX12Interface::Device->CreateDescriptorHeap(&DescriptorImGuiRender, IID_PPV_ARGS(&DirectX12Interface::DescriptorHeapImGuiRender)) !=
          S_OK)
        return oPresent(pSwapChain, SyncInterval, Flags);

      ID3D12CommandAllocator* Allocator;
      if (DirectX12Interface::Device->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(&Allocator)) != S_OK)
        return oPresent(pSwapChain, SyncInterval, Flags);

      for (size_t i = 0; i < DirectX12Interface::BuffersCounts; i++) {
        DirectX12Interface::FrameContext[i].CommandAllocator = Allocator;
      }

      if (DirectX12Interface::Device->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, Allocator, NULL,
                                                        IID_PPV_ARGS(&DirectX12Interface::CommandList)) != S_OK ||
          DirectX12Interface::CommandList->Close() != S_OK)
        return oPresent(pSwapChain, SyncInterval, Flags);

      D3D12_DESCRIPTOR_HEAP_DESC DescriptorBackBuffers;
      DescriptorBackBuffers.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
      DescriptorBackBuffers.NumDescriptors = DirectX12Interface::BuffersCounts;
      DescriptorBackBuffers.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
      DescriptorBackBuffers.NodeMask = 1;

      if (DirectX12Interface::Device->CreateDescriptorHeap(&DescriptorBackBuffers, IID_PPV_ARGS(&DirectX12Interface::DescriptorHeapBackBuffers)) !=
          S_OK)
        return oPresent(pSwapChain, SyncInterval, Flags);

      const auto RTVDescriptorSize = DirectX12Interface::Device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
      D3D12_CPU_DESCRIPTOR_HANDLE RTVHandle = DirectX12Interface::DescriptorHeapBackBuffers->GetCPUDescriptorHandleForHeapStart();

      for (size_t i = 0; i < DirectX12Interface::BuffersCounts; i++) {
        ID3D12Resource* pBackBuffer = nullptr;
        DirectX12Interface::FrameContext[i].DescriptorHandle = RTVHandle;
        pSwapChain->GetBuffer(i, IID_PPV_ARGS(&pBackBuffer));
        DirectX12Interface::Device->CreateRenderTargetView(pBackBuffer, nullptr, RTVHandle);
        DirectX12Interface::FrameContext[i].Resource = pBackBuffer;
        RTVHandle.ptr += RTVDescriptorSize;
      }

      ImGui_ImplWin32_Init(Process::Hwnd);
      ImGui_ImplDX12_Init(DirectX12Interface::Device, DirectX12Interface::BuffersCounts, DXGI_FORMAT_R8G8B8A8_UNORM,
                          DirectX12Interface::DescriptorHeapImGuiRender,
                          DirectX12Interface::DescriptorHeapImGuiRender->GetCPUDescriptorHandleForHeapStart(),
                          DirectX12Interface::DescriptorHeapImGuiRender->GetGPUDescriptorHandleForHeapStart());
      ImGui_ImplDX12_CreateDeviceObjects();
      ImGui::GetIO().ImeWindowHandle = Process::Hwnd;
      Process::WndProc = (WNDPROC)SetWindowLongPtr(Process::Hwnd, GWLP_WNDPROC, (__int3264)(LONG_PTR)WndProc);
    }
    ImGui_Initialised = true;
  }

  if (DirectX12Interface::CommandQueue == nullptr) return oPresent(pSwapChain, SyncInterval, Flags);

  if (GetAsyncKeyState(VK_HOME) & 1) ShowMenu = !ShowMenu;

  ImGui_ImplDX12_NewFrame();
  ImGui_ImplWin32_NewFrame();
  ImGui::NewFrame();
  ImGui::GetIO().MouseDrawCursor = ShowMenu;

  if (ShowMenu) {
    if (ImGui::Begin("Elden Ring Mod", &ShowMenu, ImGuiWindowFlags_AlwaysAutoResize)) {
      if (ImGui::BeginTabBar("TabBar")) {
        if (ImGui::BeginTabItem("Player")) {
          RenderPlayerTab();
          ImGui::EndTabItem();
        }
        ImGui::EndTabBar();
      }
    }
    ImGui::End();
  }
  ImGui::EndFrame();

  DirectX12Interface::_FrameContext& CurrentFrameContext = DirectX12Interface::FrameContext[pSwapChain->GetCurrentBackBufferIndex()];
  CurrentFrameContext.CommandAllocator->Reset();

  D3D12_RESOURCE_BARRIER Barrier;
  Barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
  Barrier.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
  Barrier.Transition.pResource = CurrentFrameContext.Resource;
  Barrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
  Barrier.Transition.StateBefore = D3D12_RESOURCE_STATE_PRESENT;
  Barrier.Transition.StateAfter = D3D12_RESOURCE_STATE_RENDER_TARGET;

  DirectX12Interface::CommandList->Reset(CurrentFrameContext.CommandAllocator, nullptr);
  DirectX12Interface::CommandList->ResourceBarrier(1, &Barrier);
  DirectX12Interface::CommandList->OMSetRenderTargets(1, &CurrentFrameContext.DescriptorHandle, FALSE, nullptr);
  DirectX12Interface::CommandList->SetDescriptorHeaps(1, &DirectX12Interface::DescriptorHeapImGuiRender);

  ImGui::Render();
  ImGui_ImplDX12_RenderDrawData(ImGui::GetDrawData(), DirectX12Interface::CommandList);
  Barrier.Transition.StateBefore = D3D12_RESOURCE_STATE_RENDER_TARGET;
  Barrier.Transition.StateAfter = D3D12_RESOURCE_STATE_PRESENT;
  DirectX12Interface::CommandList->ResourceBarrier(1, &Barrier);
  DirectX12Interface::CommandList->Close();
  DirectX12Interface::CommandQueue->ExecuteCommandLists(1, reinterpret_cast<ID3D12CommandList* const*>(&DirectX12Interface::CommandList));
  return oPresent(pSwapChain, SyncInterval, Flags);
}

void hkExecuteCommandLists(ID3D12CommandQueue* queue, UINT NumCommandLists, ID3D12CommandList* ppCommandLists) {
  if (!DirectX12Interface::CommandQueue) DirectX12Interface::CommandQueue = queue;

  oExecuteCommandLists(queue, NumCommandLists, ppCommandLists);
}

void APIENTRY hkDrawInstanced(ID3D12GraphicsCommandList* dCommandList, UINT VertexCountPerInstance, UINT InstanceCount, UINT StartVertexLocation,
                              UINT StartInstanceLocation) {
  return oDrawInstanced(dCommandList, VertexCountPerInstance, InstanceCount, StartVertexLocation, StartInstanceLocation);
}

DWORD WINAPI MainThread(LPVOID lpParameter) {
  bool WindowFocus = false;
  while (WindowFocus == false) {
    DWORD ForegroundWindowProcessID;
    GetWindowThreadProcessId(GetForegroundWindow(), &ForegroundWindowProcessID);
    if (GetCurrentProcessId() == ForegroundWindowProcessID) {
      Process::ID = GetCurrentProcessId();
      Process::Handle = GetCurrentProcess();
      Process::Hwnd = GetForegroundWindow();

      RECT TempRect;
      GetWindowRect(Process::Hwnd, &TempRect);
      Process::WindowWidth = TempRect.right - TempRect.left;
      Process::WindowHeight = TempRect.bottom - TempRect.top;

      char TempTitle[MAX_PATH];
      GetWindowText(Process::Hwnd, TempTitle, sizeof(TempTitle));
      Process::Title = TempTitle;

      char TempClassName[MAX_PATH];
      GetClassName(Process::Hwnd, TempClassName, sizeof(TempClassName));
      Process::ClassName = TempClassName;

      char TempPath[MAX_PATH];
      GetModuleFileNameEx(Process::Handle, NULL, TempPath, sizeof(TempPath));
      Process::Path = TempPath;

      WindowFocus = true;
    }
  }
  bool InitHook = false;
  while (InitHook == false) {
    if (DirectX12::Init() == true) {
      CreateHook(54, (void**)&oExecuteCommandLists, hkExecuteCommandLists);
      CreateHook(140, (void**)&oPresent, hkPresent);
      CreateHook(84, (void**)&oDrawInstanced, hkDrawInstanced);
      InitHook = true;
    }
  }

  Sleep(30000);
  HMODULE moduleBase = GetModuleHandleA("eldenring.exe");
  while (true) {
    if (Process::Handle) {
      handlePlayerData(&g_player, moduleBase);
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
      GetModuleFileNameA(hModule, dlldir, 512);
      for (size_t i = strlen(dlldir); i > 0; i--) {
        if (dlldir[i] == '\\') {
          dlldir[i + 1] = 0;
          break;
        }
      }
      Log("DLL STARTED");
      CreateThread(0, 0, MainThread, 0, 0, 0);
      break;
    case DLL_PROCESS_DETACH:
      FreeLibraryAndExitThread(hModule, TRUE);
      DisableAll();
      break;
    case DLL_THREAD_ATTACH:
      break;
    case DLL_THREAD_DETACH:
      break;
    default:
      break;
  }
  return TRUE;
}
