//
// Created by dsporykhin on 03.08.22.
//

#include "DisplayButtonController.h"

DisplayButtonController *DisplayButtonController::instance = nullptr;

DisplayButtonController::DisplayButtonController(HeaterController *heaterController, Display *display) {
    this->display = display;
    this->heaterController = heaterController;

    button_pressed = false;
    buttonEvent = DisplayButtonEvent::NONE;

    instance = this;

    pinMode(DISPLAY_BUTTON_PIN, INPUT_PULLUP);
    attachInterrupt(DISPLAY_BUTTON_PIN, DISPLAY_BUTTON_ISR, CHANGE);
}

void DisplayButtonController::handle() {

    if ((button_pressed) && (millis() - buttonChangedAt) > 1500) {
        button_pressed = false;
        buttonEvent = DisplayButtonEvent::LONG_CLICK;
    }

    if (buttonEvent != DisplayButtonEvent::NONE) {
        LOGGER.info("++++++  Display button event " + String(buttonEvent));
        if ((buttonEvent == DisplayButtonEvent::LONG_CLICK) &&
            ((heaterController->getTelemetryRecord()->heaterMode == STAND_BY))
            || (heaterController->getTelemetryRecord()->heaterMode == FINAL_COOLING)) {
            if (heaterController->isOxygenDoorOpenedForATime())
                heaterController->closeOxygenDoor();
            else
                heaterController->openOxygenDoorForTime(900);
        }
        buttonEvent = DisplayButtonEvent::NONE;
    }
}

void IRAM_ATTR DisplayButtonController::DISPLAY_BUTTON_ISR() {
    if (!instance)
        return;

    bool new_state = !(bool) digitalRead(DISPLAY_BUTTON_PIN);

    if ((!new_state) && (instance->button_pressed)) {
        // button was released
        if ((millis() - instance->buttonChangedAt) > 1500)
            instance->buttonEvent = DisplayButtonEvent::LONG_CLICK;
        else if ((millis() - instance->buttonChangedAt) > 50)
            instance->buttonEvent = DisplayButtonEvent::SHORT_CLICK;
    }

    instance->buttonChangedAt = millis();
    instance->button_pressed = new_state;
}