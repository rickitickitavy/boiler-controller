//
// Created by dsporykhin on 26.05.22.
//

#include "ServoController.h"
#include "Logger.h"

ServoController::ServoController(uint8_t pin, uint8_t channel, ServoHardwareSettings *servoHardwareSettings,
                                 bool inverted, float max_speed_deg_per_sec) {
    servoSettings = servoHardwareSettings;
    this->channel = channel;
    this->inverted = inverted;
    this->max_speed_deg_per_tact = max_speed_deg_per_sec / (1000.0D / SERVO_HANDLE_INTERVAL_MS);
    last_handle_time = 0;
    purpose_angle = -1;
    servo = new Servo(pin, channel);
    applySettings();
    angle = (servoHardwareSettings->working_max_angle - servoHardwareSettings->working_max_angle) / 2
            + servoHardwareSettings->working_min_angle;
    purpose_angle = angle;
    servo->setAngle(angle);
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

    purpose_angle = angle;
//    this->angle = angle;
//    servo->setAngle(angle);

    return angle;
}

void ServoController::handle() {
    if (((millis() - last_handle_time) >= SERVO_HANDLE_INTERVAL_MS)
        && (angle != purpose_angle)
            && (purpose_angle >= 0)) {
        last_handle_time = millis();

        double delta = purpose_angle - angle;
        if (abs (delta) > max_speed_deg_per_tact)
            angle += delta > 0 ? max_speed_deg_per_tact : - max_speed_deg_per_tact;
        else
            angle = purpose_angle;

        servo->setAngle(angle);
    }
}

double ServoController::getAngleGrad() {
    return servo->get_angle_grad();
}

double ServoController::setAnglePercentage(double percent) {
    if (inverted)
        percent = 100 - percent;
    return setAngle(servoSettings->working_min_angle
                    + (servoSettings->working_max_angle - servoSettings->working_min_angle)
                      * percent / 100.0);
}