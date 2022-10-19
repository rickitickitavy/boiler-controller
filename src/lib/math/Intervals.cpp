//
// Created by dsporykhin on 27.06.22.
//

#include <cstdlib>
#include <string.h>
#include <HardwareSerial.h>
#include "Intervals.h"


Intervals::Intervals(int max_intervals) {
    values = (Interval *) malloc(max_intervals * sizeof(Interval));
    this->max_intervals = max_intervals;
    stored_values_count = 0;
}

Interval* Intervals::getInterval(int interval_num) {
    if (interval_num > (stored_values_count - 1))
        interval_num = stored_values_count - 1;
    if (interval_num < 0)
        interval_num = 0;
    return &values[interval_num];
}

void Intervals::addValue(Interval* value) {
    memmove(&values[1], &values[0], (max_intervals - 1) * sizeof(Interval));
    memcpy(&values[0], value, sizeof(Interval));
    if (stored_values_count < max_intervals)
        stored_values_count++;
}

int Intervals::getCount() {
    return stored_values_count;
}
