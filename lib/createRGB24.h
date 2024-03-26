#pragma once
#include <windows.h>

extern BYTE* createRGB24fromYUY2(BYTE* pDataYUY2, UINT32 width, UINT32 height, bool mirror);
extern BYTE* createRGB24fromNV12(BYTE* pDataNV12, UINT32 width, UINT32 height, bool mirror);
