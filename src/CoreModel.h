//
// Created by dsporykhin on 27.06.22.
//

#ifndef BASE_ESP8266_MQTT_COREMODEL_H
#define BASE_ESP8266_MQTT_COREMODEL_H


#include "PumpsController.h"
#include "DoorsController.h"
#include "Telemetry.h"

class CoreModel {
private:
    double max_power;
    double angle_step;
    double angle;

    int skip_steps;

    int cycle_index;

    double core_energy_volume_dg_per_grad;

    double current_power;

    double fuel;
    bool core_heated;
    bool max_core_power_reached;

    int power_fade_out_steps;

    double energy_move_coef;

    double doors_react_ema;

    int max_power_intervals_estimated;
    double core_volume_temp;

    double radiator_stop_after_cycle;
    double radiator_normal_temperature;
    double radiator_normal_power;
    double home_temperature;
    double radiator_flow;
    double radiator_pow_coef;

    double current_core_power_doors_coef;
    double additioanl_core_doors_coef;

    PumpsController *pumpsController;
    HeaterSettings *settings;
    DoorsController *doorsController;
    Telemetry *telemetry;

    void println(String data);

public:
    double bottom_tempr;
    double lower_tempr;
    double higher_tempr;
    double top_tempr;
    double output_tempr;
    double core_tempr;

    CoreModel(HeaterSettings *settings, PumpsController *pumpsController,
              DoorsController *doorsController, Telemetry *telemetry);

    void reset();

    void handle();
};


#endif //BASE_ESP8266_MQTT_COREMODEL_H
