#pragma once

#include "includes.h"

static HANDLE pHandle;
static DWORD pid;
static uintptr_t moduleBase;
static HWND GameHWND;

uintptr_t GetModuleBaseAddress(DWORD dwProcID, const wchar_t *moduleName);
DWORD GetProcID(wchar_t *execName);

template <typename T>
T RPM(SIZE_T address)
{
    T buffer;
    ReadProcessMemory(pHandle, (LPCVOID)address, &buffer, sizeof(T), NULL);
    return buffer;
}

template <typename T>
void WPM(SIZE_T address, T buffer)
{
    WriteProcessMemory(pHandle, (LPVOID)address, &buffer, sizeof(T), NULL);
}