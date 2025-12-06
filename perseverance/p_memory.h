#pragma once
#include <Windows.h>
#include <TlHelp32.h>
#include <iostream>

class UmMemoryInstance
{
public:
    DWORD               FindPid(const std::string& processName);
    DWORD64      GetBaseAddress(const std::string& processName);

    template<typename T> T read(SIZE_T address);

    bool Attach(const std::string& processName)
    {
        p_processID = FindPid(processName);
        if (!p_processID)
            return false;

        p_handle = OpenProcess(PROCESS_ALL_ACCESS, FALSE, p_processID);
        return p_handle != nullptr;
    }

    void Detach()
    {
        if (p_handle)
        {
            CloseHandle(p_handle);
            p_handle = nullptr;
        }
        p_processID = 0;
    }

    bool IsAttached() const
    {
        return p_handle != nullptr && p_processID != 0;
    }

    DWORD GetPid() const { return p_processID; }
    HANDLE GetHandle() const { return p_handle; }

private:
    HANDLE p_handle = nullptr;
    DWORD     p_processID = 0;
};

template<typename T> 
T UmMemoryInstance::read(SIZE_T address)
{
    T buffer;
    ReadProcessMemory(p_handle, (LPCVOID)address, &buffer, sizeof(T), 0);
    return buffer;
}