//
// Created by dsporykhin on 27.06.22.
//

#include <cstdlib>
#include <string.h>
#include <HardwareSerial.h>
#include "Sma.h"


Sma::Sma(int intervals) {
    values = (double *) malloc(SMA_STORAGE_SIZE * sizeof(double));
    stored_values_count = 0;

    setIntervals(intervals);
}

double Sma::getSma() {
    return sma;
}

double Sma::addValue(double value) {
    memmove(&values[1], &values[0], (SMA_STORAGE_SIZE - 1) * sizeof(double));
    values[0] = value;
    if (stored_values_count < SMA_STORAGE_SIZE)
        stored_values_count++;

    sma = calcSma(intervals);

    return getSma();
}

double Sma::calcSma(int intervals) {
    if ((stored_values_count < intervals) && (stored_values_count > 0))
        intervals = stored_values_count;

    if (stored_values_count >= intervals){
        double sum = 0;
        for (int index = 0; index < intervals; sum += values[index++]);
        return sum / (double)intervals;
    } else
        return NAN;
}

double Sma::setIntervals(int intervals) {
    if (intervals > SMA_STORAGE_SIZE)
        intervals = SMA_STORAGE_SIZE;
    this->intervals = intervals;

    sma = calcSma(intervals);
}
