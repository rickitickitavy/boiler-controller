//
// Created by dsporykhin on 17.07.22.
//

#include <esp32-hal-gpio.h>
#include "FlowSensor.h"

FlowSensor *FlowSensor::flowSensorInstance = nullptr;

FlowSensor::FlowSensor(int pin) {
    this->pin = pin;
    pinMode(pin, INPUT_PULLDOWN);
    attachInterrupt(pin, ISR, RISING);
    flowSensorInstance = this;
}

int FlowSensor::readAndReset() {
    int current = ticks;
    ticks = 0;
    return current;
}

void IRAM_ATTR FlowSensor::ISR() {
    if (flowSensorInstance)
        flowSensorInstance->ticks++;
}