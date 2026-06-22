#pragma once

#include <windows.h>

namespace Process {
inline DWORD ID = 0;
inline HANDLE Handle = nullptr;
inline HWND Hwnd = nullptr;
inline HMODULE Module = nullptr;
inline WNDPROC WndProc = nullptr;
inline int WindowWidth = 0;
inline int WindowHeight = 0;
inline LPCSTR Title = nullptr;
inline LPCSTR ClassName = nullptr;
inline LPCSTR Path = nullptr;
}  // namespace Process
