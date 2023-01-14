//
// Created by dsporykhin on 14.01.23.
//

#ifndef BASE_ESP8266_MQTT_BUFFEREDGRAPHIC_H
#define BASE_ESP8266_MQTT_BUFFEREDGRAPHIC_H


#include <stdint.h>

#ifndef _swap_int16_t
#define _swap_int16_t(a, b)                                                    \
  {                                                                            \
    int16_t t = a;                                                             \
    a = b;                                                                     \
    b = t;                                                                     \
  }
#endif


struct FrameBuffer {
    uint16_t *buffer;
    int width;
    int height;
    FrameBuffer(uint16_t *buffer, int width, int height){
        this->buffer = buffer;
        this->width = width;
        this->height = height;
    }
};

class BufferedGraphic {
public:

    static void drawHLine(FrameBuffer &frameBuffer, uint16_t x, uint16_t y, uint16_t width, uint16_t color);

    static void drawLine(FrameBuffer &frameBuffer, uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2, uint16_t color);

    static void drawArc(FrameBuffer &frameBuffer, float cx, float cy, float r, float start_angle, float theta, int points,
    int color);

    static void fillTriangle(FrameBuffer &frameBuffer, uint16_t x0, uint16_t y0, uint16_t x1, uint16_t y1,
                             uint16_t x2, uint16_t y2, uint16_t color);

};


#endif //BASE_ESP8266_MQTT_BUFFEREDGRAPHIC_H
