//
// Created by dsporykhin on 15.01.23.
//

#include "DisplayBuffer.h"

DisplayBuffer::DisplayBuffer(int width, int height) : Adafruit_GFX(width, height) {
    buffer = (uint16_t*)malloc(2 * width * height);
}

void DisplayBuffer::freeBuffer() {
    free(buffer);
}

void DisplayBuffer::drawPixel(int16_t x, int16_t y, uint16_t color) {
    if (x > _width || y > _height)
        return;
    buffer[y * _width + x] = color;
}

void DisplayBuffer::drawFastHLine(int16_t x, int16_t y, int16_t w, uint16_t color) {
    int shift = y * _width + x;
    if ((x + w) > _width)
        w = _width - x;
    for (int ind = 0; ind < w; ind++)
        buffer[shift++] = color;
}

void DisplayBuffer::drawFastVLine(int16_t x, int16_t y, int16_t h, uint16_t color) {
    if (y >= _height)
        return;

    int shift = y * _width + x;
    if ((y + h) > _height)
        h = _height - y;
    for (int ind = 0; ind < h; ind++) {
        buffer[shift] = color;
        shift += _width;
    }
}

uint16_t  DisplayBuffer::color24To16(int color24) {
    uint16_t r = (color24 & 0x00f80000) >> 8;
    uint16_t g = (color24 & 0x0000fc00) >> 5;
    uint16_t b = (color24 & 0x000000ff) >> 3;
    return r | g | b ;
}

void DisplayBuffer::drawArc(uint16_t cx, uint16_t cy, uint16_t r, float start_angle, float theta, int points,
                            uint16_t color) {
    float px = (float)cx - ((float)r) * cos(start_angle);
    float py = (float)cy - ((float)r) * sin(start_angle);

    float dx = px - (float)cx;
    float dy = py - (float)cy;
    float ctheta = cos(theta / (points - 1));
    float stheta = sin(theta / (points - 1));
    for (int i = 1; i != points; ++i) {
        float dxtemp = ctheta * dx - stheta * dy;
        dy = stheta * dx + ctheta * dy;
        dx = dxtemp;
        drawPixel((int)(cx + dx), (int)(cy + dy),color);
    }
}

