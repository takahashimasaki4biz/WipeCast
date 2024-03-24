#include <windows.h>
#include <mfapi.h>
#include <mfidl.h>
#include <mfreadwrite.h>
#include "resource.h"
#include "lib/mutex.h"
#include "lib/camera.h"

#pragma comment(lib, "mf.lib")
#pragma comment(lib, "mfplat.lib")
#pragma comment(lib, "mfreadwrite.lib")
#pragma comment(lib, "mfuuid.lib")

PCWSTR AppName = L"WipeCast";
HWND hWnd = nullptr;
UINT32 circleSize = 400;
RECT lastrect;
CameraDevice* pCameraDevice = nullptr;
UINT32 cameraIndex = 0;

void RenderFrame(HWND hWnd, int circleSize, CameraDevice* pCameraDevice);

LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    switch (message)
    {
    case WM_CREATE:
    {
        // ウィンドウは円形に切り抜く
        HRGN hRgn = CreateEllipticRgn(0, 0, circleSize, circleSize);
        SetWindowRgn(hWnd, hRgn, TRUE);
        pCameraDevice = CameraDevice::createInstance(cameraIndex);
        if (!pCameraDevice)
            return -1;
        return 0;
    }
    case WM_ERASEBKGND:
    {
        // 背景は消去しない（ちらつき防止）
        return 1;
    }
    case WM_PAINT:
    {
        // カメラの映像を描画し続ける（WM_PAINTイベントは連続的に発生する）
        RenderFrame(hWnd, circleSize, pCameraDevice);
        return 0;
    }
    case WM_SETCURSOR:
    {
        // マウスカーソルを手のひらにする（動かせることを知らせるため）
        HCURSOR hCursor = LoadCursor(nullptr, IDC_HAND);
        SetCursor(hCursor);
        return 0;
    }
    case WM_NCHITTEST:
    {
        // ウィンドウ本体をドラッグ＆ドロップで移動できるようにする（タイトルバーを掴んだと誤認させることにより）
        LRESULT hit = DefWindowProc(hWnd, message, wParam, lParam);
        return (hit == HTCLIENT) ? HTCAPTION : hit;
    }
    case WM_WINDOWPOSCHANGING:
    {
        // 通常はウィンドウが画面上部にはみ出すと、Windowsにより画面内に強制的に戻される
        // WM_EXITSIZEMOVEイベントにて、戻される前の座標に再度戻すべく、ここで現在座標を記録しておく
        GetWindowRect(hWnd, &lastrect);
        return 0;
    }
    case WM_EXITSIZEMOVE:
    {
        // WM_WINDOWPOSCHANGINGのコメント参照
        SetWindowPos(hWnd, HWND_TOPMOST, lastrect.left, lastrect.top, (lastrect.right - lastrect.left), (lastrect.bottom - lastrect.top), SWP_SHOWWINDOW);
        return 0;
    }
    case WM_MOUSEWHEEL:
    {
        // マウスホイールで、ワイプのサイズを縮小/拡大する。タッチパッドのピンチイン/アウトでもいける
        RECT rect;
        GetWindowRect(hWnd, &rect);
        int zDelta = GET_WHEEL_DELTA_WPARAM(wParam);
        int scale = 0;
        if (zDelta > 0 && circleSize < 800)
            scale = 10;
        else if (zDelta < 0 && circleSize > 200)
            scale = -10;
        circleSize += scale * 2;
        rect.left -= scale;
        rect.right += scale;
        rect.top -= scale;
        rect.bottom += scale;
        SetWindowPos(hWnd, HWND_TOPMOST, rect.left, rect.top, (rect.right - rect.left), (rect.bottom - rect.top), SWP_SHOWWINDOW);
        HRGN hRgn = CreateEllipticRgn(0, 0, circleSize, circleSize);
        SetWindowRgn(hWnd, hRgn, TRUE); // ウィンドウを円形に切り抜く
        return 0;
    }
    case WM_NCRBUTTONDOWN:
    {
        // 中指クリックでカメラを切り替える
        for (UINT32 i = cameraIndex + 1; i <= 10; i++)
        {
            delete pCameraDevice;
            pCameraDevice = nullptr;
            cameraIndex = i % 10;
            pCameraDevice = CameraDevice::createInstance(cameraIndex);
            if (pCameraDevice)
                break;
        }
        return 0;
    }
    case WM_CHAR:
    {
        // 「q」キーで終了させる（タスクバーからウィンドウを閉じても良い）
        TCHAR chCharCode = (TCHAR)wParam;
        switch (chCharCode)
        {
        case 'q':
            PostMessage(hWnd, WM_CLOSE, 0, 0);
            break;
        }
        return 0;
    }
    case WM_DESTROY:
    {
        // 後始末
        delete pCameraDevice;
        pCameraDevice = nullptr;
        PostQuitMessage(0);
        return 0;
    }
    }
    return DefWindowProc(hWnd, message, wParam, lParam);
}

ATOM RegisterWindowClass(HINSTANCE hInstance)
{
    WNDCLASSEX wcex = {};
    wcex.cbSize = sizeof(WNDCLASSEX);
    wcex.lpfnWndProc = WndProc;
    wcex.cbClsExtra = 0;
    wcex.cbWndExtra = 0;
    wcex.hInstance = hInstance;
    wcex.hIcon = LoadIcon(GetModuleHandle(nullptr), MAKEINTRESOURCE(IDI_APP));
    wcex.hCursor = LoadCursor(nullptr, IDC_HAND);
    wcex.hbrBackground = (HBRUSH)COLOR_BACKGROUND;
    wcex.lpszMenuName = nullptr;
    wcex.lpszClassName = AppName;
    wcex.hIconSm = LoadIcon(GetModuleHandle(nullptr), MAKEINTRESOURCE(IDI_SMALL));
    return RegisterClassEx(&wcex);
}

bool InitInstance(HINSTANCE hInstance, int nCmdShow)
{
    RECT rect;
    rect.left = 0;
    rect.top = 0;
    rect.right = circleSize;
    rect.bottom = circleSize;
    hWnd = CreateWindowEx(
        WS_EX_TOPMOST,
        AppName,
        AppName,
        WS_POPUP,
        CW_USEDEFAULT, CW_USEDEFAULT, rect.right - rect.left, rect.bottom - rect.top, nullptr, nullptr, hInstance, nullptr);
    if (!hWnd)
        return false;
    ShowWindow(hWnd, nCmdShow);
    UpdateWindow(hWnd);
    return true;
}

int APIENTRY WinMain(_In_ HINSTANCE hInstance, _In_opt_ HINSTANCE hPrevInstance, _In_ LPSTR lpCmdLine, _In_ int nCmdShow)
{
    if (!CreateMyMutex(AppName))
        return 1;
    HRESULT hr = MFStartup(MF_VERSION);
    if (FAILED(hr))
        goto done;
    if (!RegisterWindowClass(hInstance))
        goto done;
    if (!InitInstance(hInstance, nCmdShow))
        goto done;
    MSG msg;
    while (GetMessage(&msg, nullptr, 0, 0))
    {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }
done:
    MFShutdown();
    ReleaseMyMutex();
    return 0;
}

void RenderFrame(HWND hWnd, int circleSize, CameraDevice* pCameraDevice)
{
    DWORD streamIndex, flags;
    CComPtr<IMFSample> pSample;
    LONGLONG llTimeStamp;

    HRESULT hr = pCameraDevice->reader()->ReadSample(MF_SOURCE_READER_FIRST_VIDEO_STREAM, 0, &streamIndex, &flags, &llTimeStamp, &pSample);
    if (SUCCEEDED(hr) && pSample && (flags & MF_SOURCE_READERF_ENDOFSTREAM) == 0)
    {
        CComPtr<IMFMediaBuffer> pBuffer;
        hr = pSample->ConvertToContiguousBuffer(&pBuffer);
        if (FAILED(hr)) { return; }
        BYTE* pData = nullptr;
        DWORD cbMaxLength, cbCurrentLength;
        hr = pBuffer->Lock(&pData, &cbMaxLength, &cbCurrentLength);
        if (FAILED(hr)) { return; }

        // ビデオフレームデータの縦横のサイズを取得する
        CComPtr<IMFMediaType> pMediaType;
        hr = pCameraDevice->reader()->GetCurrentMediaType(MF_SOURCE_READER_FIRST_VIDEO_STREAM, &pMediaType);
        if (FAILED(hr)) { return; }
        UINT32 width, height;
        hr = MFGetAttributeSize(pMediaType, MF_MT_FRAME_SIZE, &width, &height);
        if (FAILED(hr)) { return; }

        // RGB24形式に変換する（TODO:今は自力でやっているが、OpenCVあたりに任せたい）
        std::unique_ptr<BYTE> pDataRGB24;
        GUID videoSubtype = pCameraDevice->subtype();
        if (videoSubtype == MFVideoFormat_YUY2)
        {
            pDataRGB24.reset(createRGB24fromYUY2(pData, width, height));
        }
        else if (videoSubtype == MFVideoFormat_NV12)
        {
            pDataRGB24.reset(createRGB24fromNV12(pData, width, height));
        }

        // ビデオフレームデータをウィンドウに描画する
        HDC hdc = GetDC(hWnd);
        BITMAPINFOHEADER bih = {};
        bih.biSize = sizeof(BITMAPINFOHEADER);
        bih.biWidth = width;
        bih.biHeight = 0 - height;
        bih.biPlanes = 1;
        bih.biBitCount = 24;
        bih.biCompression = BI_RGB;
        UINT32 centerSize = width < height ? width : height;
        UINT32 cx = width / 2;
        UINT32 cy = height / 2;
        UINT32 up = 75; // 100以下の数値。小さいほど拡大される
        UINT32 x0 = cx - (centerSize / 2 * up / 100);
        UINT32 y0 = cy - (centerSize / 2 * up / 100);
        centerSize = centerSize * up / 100;
        SetStretchBltMode(hdc, STRETCH_DELETESCANS);
        StretchDIBits(hdc, 0, 0, circleSize, circleSize, x0, y0, centerSize, centerSize, pDataRGB24.get(), (BITMAPINFO*)&bih, DIB_RGB_COLORS, SRCCOPY);

        ReleaseDC(hWnd, hdc);
        pBuffer->Unlock();
    }
    else
    {
        // カメラから画像が取得できなかった場合の、グレー表示
        const UINT32 width = 1, height = 1;
        BYTE pDataRGB24[width * height * 3];
        pDataRGB24[0] = 0x40;
        pDataRGB24[1] = 0x40;
        pDataRGB24[2] = 0x40;

        HDC hdc = GetDC(hWnd);
        BITMAPINFOHEADER bih = {};
        bih.biSize = sizeof(BITMAPINFOHEADER);
        bih.biWidth = width;
        bih.biHeight = 0 - height;
        bih.biPlanes = 1;
        bih.biBitCount = 24;
        bih.biCompression = BI_RGB;
        UINT32 centerSize = width < height ? width : height;
        UINT32 x0 = 0;
        UINT32 y0 = 0;
        SetStretchBltMode(hdc, STRETCH_DELETESCANS);
        StretchDIBits(hdc, 0, 0, circleSize, circleSize, x0, y0, centerSize, centerSize, pDataRGB24, (BITMAPINFO*)&bih, DIB_RGB_COLORS, SRCCOPY);

        ReleaseDC(hWnd, hdc);
    }
}
