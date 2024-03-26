# include "createRGB24.h"

BYTE* createRGB24fromYUY2(BYTE* pDataYUY2, UINT32 width, UINT32 height, bool mirror) {
    BYTE* pDataRGB24 = new BYTE[width * height * 3];
    for (UINT32 y = 0; y < height; ++y) {
        for (UINT32 x = 0; x < width; x += 2) {
            int index = (y * width + x) * 2;
            int Y0 = pDataYUY2[index];
            int U = pDataYUY2[index + 1] - 128;
            int Y1 = pDataYUY2[index + 2];
            int V = pDataYUY2[index + 3] - 128;

            int R0, G0, B0, R1, G1, B1;
            // YUV to RGB conversion for the first pixel
            R0 = (298 * Y0 + 409 * V + 128) >> 8;
            G0 = (298 * Y0 - 100 * U - 208 * V + 128) >> 8;
            B0 = (298 * Y0 + 516 * U + 128) >> 8;
            // YUV to RGB conversion for the second pixel
            R1 = (298 * Y1 + 409 * V + 128) >> 8;
            G1 = (298 * Y1 - 100 * U - 208 * V + 128) >> 8;
            B1 = (298 * Y1 + 516 * U + 128) >> 8;
            // Clamping to 0 - 255
            R0 = R0 < 0 ? 0 : R0 > 255 ? 255 : R0;
            G0 = G0 < 0 ? 0 : G0 > 255 ? 255 : G0;
            B0 = B0 < 0 ? 0 : B0 > 255 ? 255 : B0;
            R1 = R1 < 0 ? 0 : R1 > 255 ? 255 : R1;
            G1 = G1 < 0 ? 0 : G1 > 255 ? 255 : G1;
            B1 = B1 < 0 ? 0 : B1 > 255 ? 255 : B1;

            // RGB storage for the first pixel
            int x2 = mirror ? ((width - 1) - x) : x;
            int o2 = mirror ? -1 : 1;
            pDataRGB24[(y * width + x2) * 3 + 2] = static_cast<BYTE>(R0);
            pDataRGB24[(y * width + x2) * 3 + 1] = static_cast<BYTE>(G0);
            pDataRGB24[(y * width + x2) * 3 + 0] = static_cast<BYTE>(B0);
            // RGB storage for the second pixel
            if (x + 1 < width) {
                pDataRGB24[(y * width + x2 + o2) * 3 + 2] = static_cast<BYTE>(R1);
                pDataRGB24[(y * width + x2 + o2) * 3 + 1] = static_cast<BYTE>(G1);
                pDataRGB24[(y * width + x2 + o2) * 3 + 0] = static_cast<BYTE>(B1);
            }
        }
    }
    return pDataRGB24;
}
