#pragma once

#include <memory>
#include <vector>
#include <atlbase.h>
#include <mfapi.h>
#include <mfidl.h>
#include <mfreadwrite.h>
#include "createRGB24.h"

class CameraDevice
{
private:
    IMFMediaSource* _pSource;
    IMFSourceReader* _pReader;
    GUID _subtype;
    CameraDevice(UINT32 cameraIndex);
public:
    static CameraDevice* createInstance(UINT32 cameraIndex);
    IMFSourceReader* reader();
    GUID subtype();
    ~CameraDevice();
};
