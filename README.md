# MiniEldenTrainer

A personal **learning project** for exploring reverse engineering, DirectX 12 hooking, and in-process memory manipulation on Windows.

> **Disclaimer:** This repository exists for **educational purposes only**. I am still learning reverse engineering and C++ systems programming, so the code quality, architecture, and safety practices are far from production-ready. Use at your own risk, and only in environments you own and control (e.g. offline single-player).

---

## About This Project

This is my first serious attempt at combining several low-level topics into one small trainer-style DLL:

- **DirectX 12 hooking** — intercepting swap chain / command queue calls to render a custom overlay
- **ImGui integration** — drawing an in-game menu on top of the rendered frame
- **Memory reading & pointer chains** — locating and updating player-related values inside a running process
- **Basic mod/trainer features** — stats display, god mode, rune editing, attribute tweaking

The target game is **Elden Ring**, but the general ideas (hooks, overlays, pointer resolution) apply to many Windows games that use D3D12.

I started this project to practice:

- How vtable / methods-table hooking works in D3D12
- How to bootstrap ImGui inside an already running game
- How to find and follow pointer chains in game memory
- How to structure a small C++ DLL project with CMake

Because I am **just getting started**, expect rough edges: hardcoded offsets, minimal error handling, experimental structure, and code that will likely change a lot as I learn more.

---

## Features (Work in Progress)

| Feature | Description |
|---------|-------------|
| In-game overlay | ImGui menu toggled with **HOME** |
| Player stats | Level, HP, FP, Stamina, Runes |
| God Mode | Keeps health / mana / stamina at max values |
| Runes editor | Add or remove runes |
| Attributes | View and increment/decrement core stats |

Game offsets are stored in `src/trainer/player_data.h` and will break after game patches — this is normal for RE-based projects.

---

## Tech Stack

- **C++20**
- **CMake** (Visual Studio 2022, x64)
- **[MinHook](https://github.com/TsudaKageyu/minhook)** — function hooking
- **[Dear ImGui](https://github.com/ocornut/imgui)** — immediate-mode UI (`imgui_impl_dx12`, `imgui_impl_win32`)
- **DirectX 12 / DXGI** — rendering hook target

---

## Project Structure

```
├── main.cpp                  # DLL entry point & main worker thread
├── src/
│   ├── core/
│   │   └── process.h         # Process / window context
│   ├── d3d12/
│   │   ├── d3d12_types.h     # Hook typedefs & original function pointers
│   │   ├── d3d12_interface.h # D3D12 device / frame context state
│   │   ├── d3d12_init.*      # Methods-table capture & MinHook setup
│   │   └── d3d12_hooks.*     # Present / ExecuteCommandLists hooks
│   ├── imgui/
│   │   └── imgui_overlay.*   # ImGuiOverlay class (menu, WndProc, render)
│   ├── trainer/
│   │   ├── player_data.h     # Player structs & game offsets
│   │   ├── memory.*          # Safe memory read & pointer chain resolver
│   │   └── player_trainer.*  # PlayerTrainer class (logic + UI tab)
│   └── utils/
│       └── logger.*          # Simple file logger
├── ImGui/                    # Dear ImGui sources
└── MinHook/                  # MinHook sources
```

---

## Building

### Requirements

- Windows 10/11 (x64)
- Visual Studio 2022 with **Desktop development with C++**
- CMake 3.20+

### Steps

```bash
mkdir build
cd build
cmake .. -G "Visual Studio 17 2022" -A x64
cmake --build . --config Release
```

Output DLL:

```
build/bin/Release/MiniEldenTrainer.dll
```

---

## Usage

1. Build the project in **Release | x64**.
2. Inject `MiniEldenTrainer.dll` into the game process using your preferred DLL injector.
3. Press **HOME** to toggle the overlay menu.

Logs are written to `EldenRingPatch.log` in the working directory of the game process.

---

## Known Limitations

- Offsets are **hardcoded** and tied to a specific game version
- No pattern scanning or signature-based address resolution yet
- Error handling is minimal
- Code structure is still evolving as I learn better practices
- Anti-cheat / online play: **do not use this online** — it is meant for offline learning only

---

## What I Want to Improve Next

- [ ] Pattern scanning instead of static offsets
- [ ] Safer memory access abstractions
- [ ] Cleaner separation between UI and game logic
- [ ] Config file for hotkeys and values
- [ ] Better shutdown / resource cleanup on DLL unload

---

## Legal & Ethical Notice

This project is for **personal education** in reverse engineering and Windows internals. It is not intended to encourage cheating in multiplayer environments or to bypass anti-cheat systems. Always respect the terms of service of the software you analyze.

---

## License

This is a personal learning repository. Third-party components (ImGui, MinHook) retain their respective licenses.

---

*If you are also learning reverse engineering — feel free to explore the code, but treat it as a student project, not a reference implementation.*
