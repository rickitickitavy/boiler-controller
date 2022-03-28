//
// Created by dsporykhin on 25.03.22.
//

#include "HeaterController.h"
#include "Defines.h"

HeaterController::HeaterController(GlobalSettings *settings,
                                   SensorController *sensorController){
    this->settings = settings;
    this->sensorController = sensorController;

    smoke_pipe_control = new Servo(SMOKE_SERVO_PIN, 0);
    oxygen_door_control = new Servo(OXYGEN_SERVO_PIN, 1);
    upper_door_control = new Servo(UPPER_SERVO_PIN, 2);
}
