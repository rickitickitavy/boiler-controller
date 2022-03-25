//
// Created by dsporykhin on 25.03.22.
//

#include "HeaterController.h"
#include "SettingsManager.h"

HeaterController::HeaterController(GlobalSettings *settings,
                                   PwmPCA9685Driver *pwmDriver,
                                   SensorController *sensorController){
    this->pwmDriver = pwmDriver;
    this->settings = settings;
    this->sensorController = sensorController;

    smoke_pipe_control = new Servo(pwmDriver, settings->smoke_pipe_control_channel_id);
    oxygen_door_control = new Servo(pwmDriver, settings->oxygen_door_control_channel_id);
    upper_door_control = new Servo(pwmDriver, settings->upper_door_control_channel_id);
}
