//
// Created by dsporykhin on 26.05.22.
//

#include "ServoController.h"
#include "Logger.h"

ServoController::ServoController(uint8_t pin, uint8_t channel, ServoHardwareSettings *servoHardwareSettings) {
    servoSettings = servoHardwareSettings;
    this->channel = channel;
    servo = new Servo(pin, channel);
    applySettings();
}

void ServoController::applySettings() {
    servo->setMinPulseLengthUs(servoSettings->min_impulse_length_us);
    servo->setMaxPulseLengthUs(servoSettings->max_impulse_length_us);
    servo->set_rotation_grad(servoSettings->total_degrees);
}

double ServoController::setAngle(double angle) {
    applySettings();
    if (angle > servoSettings->working_max_angle)
        angle = servoSettings->working_max_angle;
    if (angle < servoSettings->working_min_angle)
        angle = servoSettings->working_min_angle;

    servo->setAngle(angle);

    return angle;
}

double ServoController::getAngleGrad() {
    return servo->get_angle_grad();
}

double ServoController::setAnglePercentage(double percent) {
    return setAngle(servoSettings->working_min_angle + (servoSettings->working_max_angle - servoSettings->working_min_angle)
                    * percent / 100.0);
}