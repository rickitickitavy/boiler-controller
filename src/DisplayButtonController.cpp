//
// Created by dsporykhin on 03.08.22.
//

#include <lib/xpt2046/xpt2046.h>
#include "DisplayButtonController.h"

DisplayButtonController *DisplayButtonController::instance = nullptr;

DisplayButtonController::DisplayButtonController(HeaterController *heaterController, Display *display) {
    this->display = display;
    this->heaterController = heaterController;

    touch = new XPT2046(TOUCH_CS, TOUCH_PEN);
    touch->begin(320, 480);  // Must be done before setting rotation
    touch->setCalibration(181, 249, 1840, 1800);
    touch->setRotation(touch->ROT90);


    button_pressed = false;
    buttonEvent = DisplayButtonEvent::NONE;

    instance = this;
}

void DisplayButtonController::handle() {

    if ((button_pressed) && (millis() - buttonChangedAt) > 500) {
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

void DisplayButtonController::DISPLAY_BUTTON_ISR() {
    if (!instance)
        return;

//
//    bool new_state = !(bool) digitalRead(DISPLAY_BUTTON_PIN);
//
//    if ((!new_state) && (instance->button_pressed)) {
//        // button was released
//        if ((millis() - instance->buttonChangedAt) > 500)
//            instance->buttonEvent = DisplayButtonEvent::LONG_CLICK;
//        else if ((millis() - instance->buttonChangedAt) > 50)
//            instance->buttonEvent = DisplayButtonEvent::SHORT_CLICK;
//    }
//
//    instance->buttonChangedAt = millis();
//    instance->button_pressed = new_state;
}