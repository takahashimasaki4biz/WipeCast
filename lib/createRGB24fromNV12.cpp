# include "createRGB24.h"

BYTE* createRGB24fromNV12(BYTE* pDataNV12, UINT32 width, UINT32 height, bool mirror) {
    BYTE* pDataRGB24 = new BYTE[width * height * 3];
    const int uv_offset = width * height;
    for (UINT32 y = 0; y < height; ++y) {
        for (UINT32 x = 0; x < width; ++x) {
            int y_index = y * width + x;
            int uv_index = uv_offset + ((y / 2) * width) + (x & ~1);

            int Y = pDataNV12[y_index] - 16;
            int U = pDataNV12[uv_index] - 128;
            int V = pDataNV12[uv_index + 1] - 128;
            // YUV to RGB conversion
            int R = (298 * Y + 409 * V + 128) >> 8;
            int G = (298 * Y - 100 * U - 208 * V + 128) >> 8;
            int B = (298 * Y + 516 * U + 128) >> 8;
            // Clamping to 0 - 255
            R = R < 0 ? 0 : R > 255 ? 255 : R;
            G = G < 0 ? 0 : G > 255 ? 255 : G;
            B = B < 0 ? 0 : B > 255 ? 255 : B;
            // RGB storage
            int x2 = mirror ? ((width - 1) - x) : x;
            int rgb_index = (y * width + x2) * 3;
            pDataRGB24[rgb_index + 2] = R;
            pDataRGB24[rgb_index + 1] = G;
            pDataRGB24[rgb_index + 0] = B;
        }
    }
    return pDataRGB24;
}
