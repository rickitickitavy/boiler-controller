//
// Created by dsporykhin on 27.06.22.
//

#ifndef BASE_ESP8266_MQTT_EMA_H
#define BASE_ESP8266_MQTT_EMA_H
#define SMA_STORAGE_SIZE 32

#include <cmath>

class Sma {
private:
    float *values;
    float sma;
    int intervals;
    int stored_values_count;
public:
    Sma(int intervals);
    float addValue(float value);
    float getSma();
    bool isReady();
    float setIntervals(int intervals);
    float calcSma(int intervals);
};


#endif //BASE_ESP8266_MQTT_EMA_H
