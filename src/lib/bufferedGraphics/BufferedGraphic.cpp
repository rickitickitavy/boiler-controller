//
// Created by dsporykhin on 14.01.23.
//
#include <stdlib.h>
#include <math.h>
#include "BufferedGraphic.h"

//void BufferedGraphic::_swap_int16_t(uint16_t &op1, uint16_t &op2) {
//    uint16_t  temp = op1;
//    op1 = op2;
//    op2 =temp;
//}

void BufferedGraphic::drawHLine(FrameBuffer &frameBuffer, uint16_t x, uint16_t y, uint16_t width, uint16_t color) {
    int shift = y * frameBuffer.width + x;
    for (int ind = 0; ind < width; ind++)
        frameBuffer.buffer[shift++] = color;
}

void BufferedGraphic::drawLine(FrameBuffer &frameBuffer, uint16_t x0, uint16_t y0, uint16_t x1, uint16_t y1, uint16_t color) {
#if defined(ESP8266)
    yield();
#endif
    int16_t steep = abs(y1 - y0) > abs(x1 - x0);
    if (steep) {
        _swap_int16_t(x0, y0);
        _swap_int16_t(x1, y1);
    }

    if (x0 > x1) {
        _swap_int16_t(x0, x1);
        _swap_int16_t(y0, y1);
    }

    int16_t dx, dy;
    dx = x1 - x0;
    dy = abs(y1 - y0);

    int16_t err = dx / 2;
    int16_t ystep;

    if (y0 < y1) {
        ystep = 1;
    } else {
        ystep = -1;
    }

    for (; x0 <= x1; x0++) {
        if (steep) {
            frameBuffer.buffer[(int)x0 * frameBuffer.width + (int)y0] = color;
        } else {
            frameBuffer.buffer[(int)y0 * frameBuffer.width + (int)x0] = color;
        }
        err -= dy;
        if (err < 0) {
            y0 += ystep;
            err += dx;
        }
    }
}

void BufferedGraphic::drawArc(FrameBuffer &frameBuffer, float cx, float cy, float r, float start_angle, float theta, int points,
                    int color) {
    float px = cx - r * cos(start_angle);
    float py = cy - r * sin(start_angle);

    float dx = px - cx;
    float dy = py - cy;
    float ctheta = cos(theta / (points - 1));
    float stheta = sin(theta / (points - 1));
    for (int i = 1; i != points; ++i) {
        float dxtemp = ctheta * dx - stheta * dy;
        dy = stheta * dx + ctheta * dy;
        dx = dxtemp;
        frameBuffer.buffer[(int)(cy + dy) * frameBuffer.width + (int)(cx + dx)] = (uint16_t)color;
    }
}



void BufferedGraphic::fillTriangle(FrameBuffer &frameBuffer, uint16_t x0, uint16_t y0, uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2,
                                   uint16_t color) {
    int16_t a, b, y, last;

    // Sort coordinates by Y order (y2 >= y1 >= y0)
    if (y0 > y1) {
        _swap_int16_t(y0, y1);
        _swap_int16_t(x0, x1);
    }
    if (y1 > y2) {
        _swap_int16_t(y2, y1);
        _swap_int16_t(x2, x1);
    }
    if (y0 > y1) {
        _swap_int16_t(y0, y1);
        _swap_int16_t(x0, x1);
    }

    if (y0 == y2) { // Handle awkward all-on-same-line case as its own thing
        a = b = x0;
        if (x1 < a)
            a = x1;
        else if (x1 > b)
            b = x1;
        if (x2 < a)
            a = x2;
        else if (x2 > b)
            b = x2;
        BufferedGraphic::drawHLine(frameBuffer, a, y0, b - a + 1, color);
        return;
    }

    int16_t dx01 = x1 - x0, dy01 = y1 - y0, dx02 = x2 - x0, dy02 = y2 - y0,
            dx12 = x2 - x1, dy12 = y2 - y1;
    int32_t sa = 0, sb = 0;

    // For upper part of triangle, find scanline crossings for segments
    // 0-1 and 0-2.  If y1=y2 (flat-bottomed triangle), the scanline y1
    // is included here (and second loop will be skipped, avoiding a /0
    // error there), otherwise scanline y1 is skipped here and handled
    // in the second loop...which also avoids a /0 error here if y0=y1
    // (flat-topped triangle).
    if (y1 == y2)
        last = y1; // Include y1 scanline
    else
        last = y1 - 1; // Skip it

    for (y = y0; y <= last; y++) {
        a = x0 + sa / dy01;
        b = x0 + sb / dy02;
        sa += dx01;
        sb += dx02;
        /* longhand:
        a = x0 + (x1 - x0) * (y - y0) / (y1 - y0);
        b = x0 + (x2 - x0) * (y - y0) / (y2 - y0);
        */
        if (a > b)
            _swap_int16_t(a, b);

        BufferedGraphic::drawHLine(frameBuffer, a, y, b - a + 1, color);
    }

    // For lower part of triangle, find scanline crossings for segments
    // 0-2 and 1-2.  This loop is skipped if y1=y2.
    sa = (int32_t)dx12 * (y - y1);
    sb = (int32_t)dx02 * (y - y0);
    for (; y <= y2; y++) {
        a = x1 + sa / dy12;
        b = x0 + sb / dy02;
        sa += dx12;
        sb += dx02;
        /* longhand:
        a = x1 + (x2 - x1) * (y - y1) / (y2 - y1);
        b = x0 + (x2 - x0) * (y - y0) / (y2 - y0);
        */
        if (a > b)
            _swap_int16_t(a, b);
        BufferedGraphic::drawHLine(frameBuffer, a, y, b - a + 1, color);
    }

}
