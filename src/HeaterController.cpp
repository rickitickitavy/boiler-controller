//
// Created by dsporykhin on 25.03.22.
//

#include "HeaterController.h"

HeaterController::HeaterController(GlobalSettings *settings,
                                   SensorController *sensorController){
    this->settings = settings;
    this->sensorController = sensorController;

    smoke_pipe_control = new Servo(settings->smoke_pipe_control_channel_id);
    oxygen_door_control = new Servo(settings->oxygen_door_control_channel_id);
    upper_door_control = new Servo(settings->upper_door_control_channel_id);
}
