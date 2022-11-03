//
// Created by dsporykhin on 13.06.22.
//

#include "DoorsController.h"
#include "Defines.h"

DoorsController::DoorsController(ServosHardwareSettings *servos_hardware_settings) {
    invert_smoke = true;
    invert_oxygen = true;
    invert_upper = true;

    LOGGER.info("DoorsController starting...");
    this->servos_hardware_settings = servos_hardware_settings;
    smoke_pipe_control = new ServoController(SMOKE_SERVO_PIN, 0,
                                             &servos_hardware_settings->smoke_servo_settings, invert_smoke, 60);
    oxygen_door_control = new ServoController(OXYGEN_SERVO_PIN, 1,
                                              &servos_hardware_settings->oxygen_servo_settings, invert_oxygen, 60);
    upper_door_control = new ServoController(UPPER_SERVO_PIN, 2,
                                             &servos_hardware_settings->upper_door_servo_settings, invert_upper, 60);

    smoke_pipe_value = 66;
    oxygen_door_value = 0;
    upper_door_value = 0;

    door_opened = false;

    applyStatus();
    LOGGER.info("DoorsController started...");
}

void DoorsController::applyStatus() {
    if (door_opened){
        smoke_pipe_control->setAnglePercentage(100);
        oxygen_door_control->setAnglePercentage(0, true);
        upper_door_control->setAnglePercentage(100);
    } else {
        smoke_pipe_control->setAnglePercentage(smoke_pipe_value);
        oxygen_door_control->setAnglePercentage(oxygen_door_value);
        upper_door_control->setAnglePercentage(upper_door_value);
    }
}

void DoorsController::setDoorOpened(bool door_opened) {
    bool old_opened = this->door_opened;
    this->door_opened = door_opened;
    if (old_opened != door_opened)
        applyStatus();
}

void DoorsController::setOxygenDoorValue(double value) {
    if (isnan(value))
        value = 0;
    oxygen_door_value = value;
    applyStatus();
}

double DoorsController::getOxygenDoorValue() {
    return oxygen_door_value;
}

void DoorsController::setSmokePipeValue(double value) {
    if (isnan(value))
        value = 0;
    smoke_pipe_value = value;
    applyStatus();
}

double DoorsController::getSmokePipeValue() {
    return smoke_pipe_value;
}


void DoorsController::setUpperDoorValue(double value) {
    if (isnan(value))
        value = 0;
    upper_door_value = value;
    applyStatus();
}

double DoorsController::getUpperDoorValue() {
    return upper_door_value;
}

double DoorsController::getSmokePipeAngle() {
    return smoke_pipe_control->getAngleGrad();
}


double DoorsController::getOxygenDoorAngle() {
    return oxygen_door_control->getAngleGrad();
}

double DoorsController::getUpperDoorAngle() {
    return upper_door_control->getAngleGrad();
}

bool DoorsController::isDoorOpened() {
    return door_opened;
}

void DoorsController::handle() {
    smoke_pipe_control->handle();
    upper_door_control->handle();
    oxygen_door_control->handle();
}