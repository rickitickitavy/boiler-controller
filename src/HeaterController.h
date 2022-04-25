//
// Created by dsporykhin on 25.03.22.
//

#ifndef BASE_ESP8266_MQTT_HEATERCONTROLLER_H
#define BASE_ESP8266_MQTT_HEATERCONTROLLER_H


#include <cstdint>
#include "lib/servo/Servo.h"
#include "GlobalSettings.h"
#include "SensorController.h"
#include "PID.h"

#define CORE_SENSOR_INDEX 0

class HeaterController {
private:
    GlobalSettings *settings;
    SensorController *sensorController;
    SettingsNavigator *settingsNavigator;

    Servo *smoke_pipe_control;
    Servo *oxygen_door_control;
    Servo *upper_door_control;

    PID *oxygen_pid;

    long last_cycle_time;

public:
    HeaterController(GlobalSettings *settings,
                     SensorController *sensorController, SettingsNavigator *settingsNavigator);

    void handle();
};


#endif //BASE_ESP8266_MQTT_HEATERCONTROLLER_H
