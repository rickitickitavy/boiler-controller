//
// Created by dsporykhin on 13.02.21.
//
#include <Arduino.h>
#include <ArduinoOTA.h>
#include "WiFiController.h"
#include "MqttClient.h"
#include "Switcher.h"

long lastWork;

SettingsManager *settingsManager;
WiFiController *wiFiController;
MqttClient *mqtt;
Switcher *switcher;

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

    mqtt = new MqttClient(settingsManager->getSettings());

    lastWork = millis();

    LOGGER.info("start device");

    switcher = new Switcher(settingsManager->getSettings(), mqtt);

    LOGGER.info("all done");
}


void loop(){
    ArduinoOTA.handle();
    mqtt->dispatch();
    switcher->dispatch();
}