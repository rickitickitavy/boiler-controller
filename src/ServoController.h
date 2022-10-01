//
// Created by dsporykhin on 26.05.22.
//

#ifndef BASE_ESP8266_MQTT_SERVOCONTROLLER_H
#define BASE_ESP8266_MQTT_SERVOCONTROLLER_H


#include <lib/servo/Servo.h>
#include "HeaterSettings.h"

class ServoController {
private:
    ServoHardwareSettings *servoSettings;
    Servo *servo;
    uint8_t channel;

    void applySettings();
public:
    ServoController(uint8_t pin, uint8_t channel, ServoHardwareSettings *servoHardwareSettings, bool inverted);

    double setAngle(double angle);
    double getAngleGrad();

    double setAnglePercentage(double percent);

    bool inverted;
};


#endif //BASE_ESP8266_MQTT_SERVOCONTROLLER_H
