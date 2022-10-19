//
// Created by dsporykhin on 27.06.22.
//

#ifndef BASE_ESP8266_MQTT_INTERVALS_H
#define BASE_ESP8266_MQTT_INTERVALS_H
#define SMA_STORAGE_SIZE 64

#include <cmath>

struct Interval{
    long time;
    double value;
};

class Intervals {
private:
    Interval *values;
    int max_intervals;
    int stored_values_count;
public:
    Intervals(int max_intervals);
    void addValue(Interval *value);
    Interval *getInterval(int interval_num);
    int getCount();
};


#endif //BASE_ESP8266_MQTT_INTERVALS_H
