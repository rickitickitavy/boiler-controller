//
// Created by dsporykhin on 25.03.22.
//

#ifndef BASE_ESP8266_MQTT_HEATERCONTROLLER_H
#define BASE_ESP8266_MQTT_HEATERCONTROLLER_H


#include <drivers/PwmPCA9685Driver.h>
#include <cstdint>
#include "drivers/Servo.h"
#include "GlobalSettings.h"
#include "SensorController.h"

class HeaterController {
private:
    GlobalSettings *settings;
    PwmPCA9685Driver *pwmDriver;
    SensorController *sensorController;

    Servo *smoke_pipe_control;
    Servo *oxygen_door_control;
    Servo *upper_door_control;

public:
    HeaterController(GlobalSettings *settings,
                     PwmPCA9685Driver *pwmDriver,
                     SensorController *sensorController);
};


#endif //BASE_ESP8266_MQTT_HEATERCONTROLLER_H
