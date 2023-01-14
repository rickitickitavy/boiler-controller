//
// Created by dsporykhin on 14.01.23.
//

#ifndef BASE_ESP8266_MQTT_BUFFEREDGRAPHIC_H
#define BASE_ESP8266_MQTT_BUFFEREDGRAPHIC_H


#include <stdint.h>

struct FrameBuffer {
    uint16_t *buffer;
    int width;
    int height;
};

class BufferedGraphic {
private:
    static void _swap_int16_t(uint16_t &op1, uint16_t &op2);

public:
    static void drawLine(FrameBuffer &frameBuffer, uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2, uint16_t color);

    static void fillTriangle(int16_t x0, int16_t y0, int16_t x1, int16_t y1,
                             int16_t x2, int16_t y2, uint16_t color);

};


#endif //BASE_ESP8266_MQTT_BUFFEREDGRAPHIC_H
