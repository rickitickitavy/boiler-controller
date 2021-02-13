//
// Created by dsporykhin on 13.02.21.
//
#include <Arduino.h>
#include <ArduinoOTA.h>
#include "WiFiController.h"

long lastWork;

SettingsManager *settingsManager;
WiFiController *wiFiController;

void setup(){
#ifdef CON_DEBUG
    Serial.begin(74880);
    Serial.println("---");

    LOGGER.info("Started UART at 921600");
#endif
    LOGGER.info("Starting...");

    ArduinoOTA.begin(true);

    settingsManager = new SettingsManager();

    wiFiController = new WiFiController(settingsManager);

    lastWork = millis();
    LOGGER.info("all done");
}


void loop(){

    ArduinoOTA.handle();

}