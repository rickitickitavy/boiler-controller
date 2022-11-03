//
// Created by dsporykhin on 26.05.22.
//

#ifndef BASE_ESP8266_MQTT_SERVOCONTROLLER_H
#define BASE_ESP8266_MQTT_SERVOCONTROLLER_H


#include <lib/servo/Servo.h>
#include "HeaterSettings.h"

#define SERVO_HANDLE_INTERVAL_MS 20.0D

class ServoController {
private:
    ServoHardwareSettings *servoSettings;
    Servo *servo;
    uint8_t channel;
    float max_speed_deg_per_tact;

    double purpose_angle;
    double angle;
    long last_handle_time;

    void applySettings();
public:
    ServoController(uint8_t pin, uint8_t channel, ServoHardwareSettings *servoHardwareSettings, bool inverted, float max_speed_deg_per_sec);

    double setAngle(double angle, bool now);
    double getAngleGrad();

    double setAnglePercentage(double percent, bool now);
    double setAnglePercentage(double percent);

    void handle();

    bool inverted;
};


#endif //BASE_ESP8266_MQTT_SERVOCONTROLLER_H
