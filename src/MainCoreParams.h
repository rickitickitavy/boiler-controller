//
// Created by dsporykhin on 12.06.22.
//

#ifndef BASE_ESP8266_MQTT_MAINCOREPARAMS_H
#define BASE_ESP8266_MQTT_MAINCOREPARAMS_H

struct MainCoreParams{
    double core_DEMA_temperature;
    double current_core_power;
    double core_EMA_power;
    double core_DEMA_power;
    long DiffEMA_down_bellow_zero_at;
    long DiffEMA_rose_above_zero_at;
    double core_flow;

    double core_temperature;
};


#endif //BASE_ESP8266_MQTT_MAINCOREPARAMS_H
