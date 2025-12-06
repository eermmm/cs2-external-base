#include "p_memory.h"

DWORD UmMemoryInstance::FindPid(const std::string& processName)
{
    PROCESSENTRY32 pt;
    HANDLE hsnap = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
    pt.dwSize = sizeof(PROCESSENTRY32);
    if (Process32First(hsnap, &pt)) {
        do {
            if (!lstrcmpi(pt.szExeFile, processName.c_str())) {
                CloseHandle(hsnap);
                this->p_processID = pt.th32ProcessID;
                return pt.th32ProcessID;
            }
        } while (Process32Next(hsnap, &pt));
    }
    CloseHandle(hsnap);
    return { NULL };
}

DWORD64 UmMemoryInstance::GetBaseAddress(const std::string& processName)
{
    HANDLE hSnap = CreateToolhelp32Snapshot(TH32CS_SNAPMODULE | TH32CS_SNAPMODULE32, p_processID);
    if (hSnap == INVALID_HANDLE_VALUE)
        return 0;

    MODULEENTRY32 modEntry{};
    modEntry.dwSize = sizeof(modEntry);

    if (Module32First(hSnap, &modEntry)) {
        do {
            if (!strcmp(modEntry.szModule, processName.c_str())) {
                CloseHandle(hSnap);
                return reinterpret_cast<uintptr_t>(modEntry.modBaseAddr);
            }
        } while (Module32Next(hSnap, &modEntry));
    }

    CloseHandle(hSnap);
    return 0;
}
