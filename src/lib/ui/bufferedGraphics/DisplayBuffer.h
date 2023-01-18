//
// Created by dsporykhin on 15.01.23.
//

#ifndef BASE_ESP8266_MQTT_DISPLAYBUFFER_H
#define BASE_ESP8266_MQTT_DISPLAYBUFFER_H


#include <lib/adafruit/Adafruit_GFX.h>

class DisplayBuffer : public Adafruit_GFX {
private:
public:
    uint16_t *buffer;

    DisplayBuffer(int width, int height);

    static uint16_t color24To16(int color24);

    void freeBuffer(),
        drawPixel(int16_t x, int16_t y, uint16_t color) override,
        drawFastVLine(int16_t x, int16_t y, int16_t h, uint16_t color) override,
        drawFastHLine(int16_t x, int16_t y, int16_t w, uint16_t color) override,
        drawArc(uint16_t cx, uint16_t cy, uint16_t r, float start_angle, float theta, int points, uint16_t color),
        fillScreen(uint16_t color) override;
    uint16_t calcTextWidth(const char *msg);





};


#endif //BASE_ESP8266_MQTT_DISPLAYBUFFER_H
