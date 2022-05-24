//
// Created by dsporykhin on 24.05.22.
//

#ifndef BASE_ESP8266_MQTT_PUMPSCONTROLLER_H
#define BASE_ESP8266_MQTT_PUMPSCONTROLLER_H


#include <cstdint>
#include "HeaterSettings.h"
#include "PumpController.h"

class PumpsController {
private:
    int count_of_on_pumps;
    uint8_t switch_index;
public:
    HeaterSettings *heaterSettings;

    PumpController *pump1;
    PumpController *pump2;

    PumpsController(HeaterSettings *heaterSettings);

    int setOnPumpsCount(uint8_t onPumpsCount);
};


#endif //BASE_ESP8266_MQTT_PUMPSCONTROLLER_H
