//
// Created by dsporykhin on 03.08.22.
//

#ifndef BASE_ESP8266_MQTT_DISPLAYBUTTONCONTROLLER_H
#define BASE_ESP8266_MQTT_DISPLAYBUTTONCONTROLLER_H

#include <HeaterController.h>
#include <Display.h>
#include "lib/xpt2046/xpt2046.h"

enum DisplayButtonEvent{
    NONE = 0,
    SHORT_CLICK = 1,
    LONG_CLICK = 2
};


class DisplayButtonController {
private:
    static DisplayButtonController *instance;
    HeaterController *heaterController;
    Display *display;
    XPT2046 *touch;

    long buttonChangedAt;
    bool button_pressed;
    DisplayButtonEvent buttonEvent;

    static void DISPLAY_BUTTON_ISR();

public:
    DisplayButtonController(HeaterController *heaterController, Display *display);
    void handle();
};


#endif //BASE_ESP8266_MQTT_DISPLAYBUTTONCONTROLLER_H
