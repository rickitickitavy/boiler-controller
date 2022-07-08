//
// Created by dsporykhin on 24.04.22.
//

#include "PidRegulator.h"

PidRegulator::PidRegulator(HeaterSettings *settings, SensorController *sensorController, MainCoreParams *mainCoreParams) {
    this->settings = settings;
    this->sensorController = sensorController;
    this->mainCoreParams = mainCoreParams;

    i = 0;
    prior_value = 0;
}

void PidRegulator::handle() {

//    double value = sensorController->getSmaValue(T_SENS_INDEX_CORE) - prior_value;

    double _core_temperature = sensorController->getNotNANSmaValue(T_SENS_INDEX_CORE);

    if (!prior_value)
        prior_value = _core_temperature;

    p = settings->oxygen_pid.p * (settings->temperatureSettings.core_target - _core_temperature);

    d = settings->oxygen_pid.d * (prior_value - _core_temperature);

    i += (settings->temperatureSettings.core_target - _core_temperature) * settings->oxygen_pid.i;

    if (i > settings->oxygen_pid.max_i)
        i = settings->oxygen_pid.max_i;
    else if (i < settings->oxygen_pid.min_i)
        i = settings->oxygen_pid.min_i;

    output_raw_value = p + d + i;

    output_value_prcnt = output_raw_value;
    if (output_value_prcnt > 100)
        output_value_prcnt = 100;
    else if (output_value_prcnt < 0)
        output_value_prcnt = 0;

    prior_value = _core_temperature;
}

double PidRegulator::getRawValue() {
    return output_raw_value;
}

double PidRegulator::getValuePrcnt() {
    return output_value_prcnt;
}

void PidRegulator::fillPID(float &p, float &i, float &d, float &raw_value, float &value) {
    p = (float) this->p;
    i = (float) this->i;
    d = (float) this->d;
    raw_value = this->output_raw_value;
    value = (float) this->output_value_prcnt;
}