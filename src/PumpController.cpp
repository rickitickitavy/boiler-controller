//
// Created by dsporykhin on 24.05.22.
//

#include <esp32-hal-gpio.h>
#include "PumpController.h"

PumpController::PumpController(uint8_t pin) {
    this->pin = pin;
    pinMode(pin, OUTPUT);
    manual_on = false;
    stateOn = false;
    digitalWrite(pin, LOW);
}

bool PumpController::isStateOn() {
    return stateOn;
}

bool PumpController::isManualOn() {
    return manual_on;
}

void PumpController::setManualOn(bool manual_on) {
    this->manual_on = manual_on;
    stateToPin();
}

bool PumpController::setStateOn(bool new_state) {
    stateOn = new_state;
    stateToPin();
}

void PumpController::stateToPin() {
    uint8_t output_state = manual_on || stateOn
                           ? HIGH : LOW;
    digitalWrite(pin, output_state);
}

StateName PumpController::getStateName() {
    return (StateName)((stateOn & 1) | ((manual_on & 1) << 1));
}

bool PumpController::isOn() {
    return manual_on || stateOn;
}