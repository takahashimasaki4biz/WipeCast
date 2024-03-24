#include "mutex.h"

HANDLE hMutex = nullptr;
bool CreateMyMutex(PCWSTR mutexName)
{
    hMutex = CreateMutex(nullptr, FALSE, mutexName);
    if (hMutex == nullptr)
        return false;
    if (GetLastError() == ERROR_ALREADY_EXISTS)
    {
        CloseHandle(hMutex);
        hMutex = nullptr;
        return false;
    }
    return true;
}

void ReleaseMyMutex()
{
    if (hMutex != nullptr)
    {
        ReleaseMutex(hMutex);
        CloseHandle(hMutex);
        hMutex = nullptr;
    }
}
