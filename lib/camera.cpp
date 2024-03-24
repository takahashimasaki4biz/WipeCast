#include "camera.h"

bool getDevices(IMFAttributes** pAttributes, IMFActivate*** ppDevices, UINT32* count)
{
    *ppDevices = nullptr;
    *count = 0;

    HRESULT hr = MFCreateAttributes(pAttributes, 1);
    if (FAILED(hr)) { throw 0; }

    hr = (*pAttributes)->SetGUID(MF_DEVSOURCE_ATTRIBUTE_SOURCE_TYPE, MF_DEVSOURCE_ATTRIBUTE_SOURCE_TYPE_VIDCAP_GUID);
    if (FAILED(hr)) { return false; }

    hr = MFEnumDeviceSources(*pAttributes, ppDevices, count);
    if (FAILED(hr) || *count < 1) { return false; }

    return true;
}

void releaseDevices(IMFAttributes* pAttributes, IMFActivate** ppDevices, UINT32 count)
{
    if (pAttributes)
    {
        pAttributes->Release();
    }
    for (UINT32 i = 0; i < count; i++)
    {
        if (ppDevices[i])
        {
            ppDevices[i]->Release();
            ppDevices[i] = nullptr;
        }
    }
    CoTaskMemFree(ppDevices);
}

CameraDevice::CameraDevice(UINT32 cameraIndex)
{
    _pSource = nullptr;
    _pReader = nullptr;
    _subtype = GUID_NULL;

    IMFAttributes* pAttributes = nullptr;
    IMFActivate** ppDevices = nullptr;
    UINT32 count = 0;
    if (!getDevices(&pAttributes, &ppDevices, &count)) { throw 0; }

    if (cameraIndex >= count) { releaseDevices(pAttributes, ppDevices, count); throw 0; }

    HRESULT hr = ppDevices[cameraIndex]->ActivateObject(__uuidof(IMFMediaSource), (void**)&_pSource);
    if (FAILED(hr)) { releaseDevices(pAttributes, ppDevices, count); throw 0; }

    hr = MFCreateSourceReaderFromMediaSource(_pSource, pAttributes, &_pReader);
    if (FAILED(hr)) { releaseDevices(pAttributes, ppDevices, count); throw 0; }

    CComPtr<IMFMediaType> pMediaType;
    hr = _pReader->GetCurrentMediaType(MF_SOURCE_READER_FIRST_VIDEO_STREAM, &pMediaType);
    if (FAILED(hr)) { releaseDevices(pAttributes, ppDevices, count); throw 0; }
    pMediaType->GetGUID(MF_MT_SUBTYPE, &_subtype);
    if (_subtype != MFVideoFormat_YUY2 &&
        _subtype != MFVideoFormat_NV12)
    {
        releaseDevices(pAttributes, ppDevices, count);
        throw 0;
    }

    releaseDevices(pAttributes, ppDevices, count);
}

CameraDevice* CameraDevice::createInstance(UINT32 cameraIndex)
{
    try
    {
        return new CameraDevice(cameraIndex);
    }
    catch (...)
    {
        return nullptr;
    }
}

IMFSourceReader* CameraDevice::reader()
{
    return _pReader;
}

GUID CameraDevice::subtype()
{
    return _subtype;
}

CameraDevice::~CameraDevice()
{
    if (_pReader)
    {
        _pReader->Release();
        _pReader = nullptr;
    }
    if (_pSource)
    {
        _pSource->Shutdown();
        _pSource->Release();
        _pSource = nullptr;
    }
}
