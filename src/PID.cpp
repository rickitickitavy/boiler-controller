//
// Created by dsporykhin on 24.04.22.
//

#include "PID.h"

PID::PID(SettingsNavigator *settingsNavigator, PidSettings *settings, SensorController *sensorController,
         int sensor_index, double *target_value) {
    this->settingsNavigator = settingsNavigator;
    this->settings = settings;
    this->sensorController = sensorController;
    this->sensor_index = sensor_index;
    this->target_value = target_value;

    i_value = 0;
    prior_value = 0;
}

void PID::handle() {

    double value = sensorController->sensor_data[sensor_index].value - prior_value;

    if ((prior_value == 0) || on_hold)
        prior_value = value;
    on_hold = false;

    double d = settings->d * (value - prior_value);

    double p = settings->p * *target_value;

    i_value += settings->i * (*target_value - value);
    if (i_value > settings->max_i)
        i_value = settings->max_i;
    else if (i_value < settings->min_i)
        i_value = settings->min_i;

    prior_value = (prior_value * (settings->d_sma - 1) + value) / settings->d_sma;
    output_raw_value = p + d + i_value;

    // calc scale
    double scaler = p + settings->max_i - settings->min_i; // min_i always is less or equal zero

    output_value_prcnt = output_raw_value * 100 / scaler;

    if (output_value_prcnt > settings->max_output_value_prcnt)
        output_value_prcnt = settings->max_output_value_prcnt;
    else if (output_value_prcnt < settings->min_output_value_prcnt)
        output_value_prcnt = settings->min_output_value_prcnt;
}

double PID::getRawValue() {
    return output_raw_value;
}

double PID::getValuePrcnt() {
    return output_value_prcnt;
}

void PID::hold() {
    on_hold = true;
}