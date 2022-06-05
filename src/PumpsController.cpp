//
// Created by dsporykhin on 24.05.22.
//

#include "PumpsController.h"
#include "Defines.h"

PumpsController::PumpsController(HeaterSettings *heaterSettings) {
    this->heaterSettings = heaterSettings;
    // initiate pump1
    pump1 = new PumpController(PUMP_1_PIN);

    // initiate pump2 if present
    pump2 = new PumpController(PUMP_2_PIN);

    switch_index = 0;
}

int PumpsController::setOnPumpsCount(uint8_t onPumpsCount) {
    count_of_on_pumps = onPumpsCount;

    if (!heaterSettings->two_pumps_settings.enabled) {
        pump2->setManualOn(false);
        pump2->setStateOn(false);
        // restrict required count of pumps by 1 if second pump is disabled
        if (count_of_on_pumps > 1)
            count_of_on_pumps = 1;

    }

    if (!count_of_on_pumps) {
        pump1->setStateOn(LOW);
        pump2->setStateOn(LOW);
    } else if (count_of_on_pumps == 1) {
        // only 1 pump required. find which of pumps currently is on
        if (pump1->isManualOn() || pump2->isManualOn()) {
            // one of pumps or both are manually switched ON
            if (pump1->isManualOn()) {
                // pump 1 is manually ON
                pump1->setStateOn(true);
                pump2->setStateOn(false);
            } else {
                // pump 2 is manually ON
                pump2->setStateOn(true);
                pump1->setStateOn(false);
            }
        } else
            // no one pumps is manually ON. Check which is on
        if (pump1->isStateOn() || pump2->isStateOn()) {
            // it is one of them or both are switched ON then switch OFF second pump
            if (pump1->isStateOn())
                pump2->setStateOn(false);
            else
                pump1->setStateOn(false);
        } else {
            // no one pump is switched ON. switch ON next. round robbin pumps
            if ((switch_index++) & 1)
                pump1->setStateOn(true);
            else
                pump2->setStateOn(true);
        }

    } else {
        // two pumps must be switched ON
        pump1->setStateOn(true);
        pump2->setStateOn(true);
    }

    // return count of working pumps
    return (((uint8_t)pump1->isOn()) & 01) + (((uint8_t)pump1->isOn()) & 01);
}

int PumpsController::getOnPumpsCount() {
    return (pump1->isOn() ? 1 : 0) + (pump2->isOn() ? 1 : 0);
}