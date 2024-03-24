#pragma once
#include <windows.h>

extern bool CreateMyMutex(PCWSTR mutexName);
extern void ReleaseMyMutex();
