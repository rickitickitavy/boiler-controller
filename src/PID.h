//
// Created by dsporykhin on 24.04.22.
//

#ifndef BASE_ESP8266_MQTT_PID_H
#define BASE_ESP8266_MQTT_PID_H


#include "GlobalSettings.h"
#include "SettingsNavigator.h"
#include "SensorController.h"

class PID {
private:
    double i_sum;
    double prior_value;
    double output_raw_value;
    double output_value_prcnt;

    double p, i, d;

    boolean on_hold;

    int sensor_index;
    double *target_value;

    PidSettings *settings;
    SettingsNavigator *settingsNavigator;
    SensorController *sensorController;

public:
    PID(SettingsNavigator *settingsNavigator
            , PidSettings *settings
            , SensorController *sensorController
            , int sensor_index
            , double *target_value);
    void handle();
    double getRawValue();
    double getValuePrcnt();

    void fillPID(float &p, float &i, float &i_sum, float &d, bool &on_hold, float &prior_value);

    void hold();

};


#endif //BASE_ESP8266_MQTT_PID_H
