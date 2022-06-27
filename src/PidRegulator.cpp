//
// Created by dsporykhin on 24.04.22.
//

#include "PidRegulator.h"

PidRegulator::PidRegulator(HeaterSettings *settings, SensorController *sensorController, MainCoreParams *mainCoreParams) {
    this->settings = settings;
    this->sensorController = sensorController;
    this->mainCoreParams = mainCoreParams;

    i_sum = 0;
    prior_value = 0;
}

void PidRegulator::handle() {

    double value = sensorController->getSmaValue(T_SENS_INDEX_CORE) - prior_value;

    if ((prior_value == 0) || on_hold)
        prior_value = value;
    on_hold = false;

    d = settings->oxygen_pid.d * (value - prior_value);

    p = settings->oxygen_pid.p * settings->temperatureSettings.core_target;

    i_sum += settings->oxygen_pid.i * (settings->temperatureSettings.core_target - value);
    if (i_sum > settings->oxygen_pid.max_i)
        i_sum = settings->oxygen_pid.max_i;
    else if (i_sum < settings->oxygen_pid.min_i)
        i_sum = settings->oxygen_pid.min_i;

    prior_value = (prior_value * (settings->oxygen_pid.d_sma - 1) + value) / settings->oxygen_pid.d_sma;
    output_raw_value = p + d + i_sum;

    // calc scale
    double scaler = p + settings->oxygen_pid.max_i - settings->oxygen_pid.min_i; // min_i always is less or equal zero

    output_value_prcnt = output_raw_value * 100 / scaler;

    if (output_value_prcnt > settings->oxygen_pid.max_output_value_prcnt)
        output_value_prcnt = settings->oxygen_pid.max_output_value_prcnt;
    else if (output_value_prcnt < settings->oxygen_pid.min_output_value_prcnt)
        output_value_prcnt = settings->oxygen_pid.min_output_value_prcnt;
}

double PidRegulator::getRawValue() {
    return output_raw_value;
}

double PidRegulator::getValuePrcnt() {
    return output_value_prcnt;
}

void PidRegulator::hold() {
    on_hold = true;
}

void PidRegulator::fillPID(float &p, float &i, float &i_sum, float &d, bool &on_hold, float &prior_value) {
    p = (float) this->p;
    i = (float) this->i;
    i_sum = (float) this->i_sum;
    d = (float) this->d;
    on_hold = this->on_hold;
    prior_value = (float) this->prior_value;
}