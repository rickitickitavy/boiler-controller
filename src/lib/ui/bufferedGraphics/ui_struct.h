//
// Created by dsporykhin on 18.01.23.
//

#ifndef BASE_ESP8266_MQTT_UI_STRUCT_H
#define BASE_ESP8266_MQTT_UI_STRUCT_H

#include <stdint.h>
#include "DisplayBuffer.h"


struct ColorPart {
    uint16_t color;
    float ends_at;
    ColorPart *next; // if next is NULL then this part extends to the end
    ColorPart(int color,
              float ends_at,
              ColorPart *next) {
        this->next = next;
        this->ends_at = ends_at;
        this->color = DisplayBuffer::color24To16(color);
    }
};
#endif //BASE_ESP8266_MQTT_UI_STRUCT_H
