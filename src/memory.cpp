#include "../include/memory.h"

uintptr_t GetModuleBaseAddress(DWORD dwProcID, const wchar_t *moduleName)
{
    uintptr_t modBaseAddr = 0;
    MODULEENTRY32 moduleEntry = {0};
    HANDLE hSnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPMODULE | TH32CS_SNAPMODULE32, dwProcID);

    if (hSnapshot != INVALID_HANDLE_VALUE)
    {
        moduleEntry.dwSize = sizeof(MODULEENTRY32);
        if (Module32First(hSnapshot, &moduleEntry))
        {
            do
            {
                if (!wcscmp((wchar_t *)moduleEntry.szModule, (wchar_t *)moduleName))
                {
                    modBaseAddr = (uintptr_t)moduleEntry.modBaseAddr;
                    break;
                }

            } while (Module32Next(hSnapshot, &moduleEntry));
        }
        CloseHandle(hSnapshot);
    }
    return modBaseAddr;
}

DWORD GetProcID(wchar_t *execName)
{
    PROCESSENTRY32 procEntry = {0};
    HANDLE hSnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, NULL);

    if (!hSnapshot)
        return 0;

    procEntry.dwSize = sizeof(procEntry);

    if (!Process32First(hSnapshot, &procEntry))
        return 0;

    do
    {
        if (!wcscmp((wchar_t *)procEntry.szExeFile, execName))
        {
            CloseHandle(hSnapshot);
            return procEntry.th32ProcessID;
        }
    } while (Process32Next(hSnapshot, &procEntry));

    CloseHandle(hSnapshot);
    return 0;
}