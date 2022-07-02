//
// Created by dsporykhin on 27.06.22.
//

#ifndef BASE_ESP8266_MQTT_COREMODEL_H
#define BASE_ESP8266_MQTT_COREMODEL_H


#include "PumpsController.h"

class CoreModel {
private:
    double max_power;
    double angle_step;
    double angle;
    int skip_steps;

    double core_energy_volume_dg_per_grad;

    double current_power;


    double energy_move_coef;

    int max_power_intervals_estimated;
    double core_volume_temp;


    PumpsController *pumpsController;
    HeaterSettings *settings;

public:
    double bottom_tempr;
    double lower_tempr;
    double higher_tempr;
    double top_tempr;
    double output_tempr;
    double core_tempr;

    CoreModel(HeaterSettings *settings, PumpsController *pumpsController,
              double angle_step, double max_power, double max_power_intervals,
              double tempr_coef, int skip_steps);

    void handle();
};


#endif //BASE_ESP8266_MQTT_COREMODEL_H
