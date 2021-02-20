//
// Created by dsporykhin on 13.02.21.
//
#include <Arduino.h>
#include <ArduinoOTA.h>
#include "WiFiController.h"
#include "MqttClient.h"
#include "Switcher.h"

//#include <ESP8266WiFi.h>

long lastWork;
long lastWorkA;

SettingsManager *settingsManager;
WiFiController *wiFiController;
MqttClient *mqtt;
Switcher *switcher;

void setup(){
//WiFi.begin("TP-LINK_B1C6", "15121820");
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
    lastWorkA = millis();

    LOGGER.info("start device");

    switcher = new Switcher(settingsManager->getSettings(), mqtt);

    LOGGER.info("all done");
}


void loop(){
    ArduinoOTA.handle();
    mqtt->dispatch();
    switcher->dispatch();
//    if (millis() > 12000){
//        if (millis() - lastWorkA > 300){
//            lastWorkA = millis();
//            LOGGER.info(String(digitalRead(0)));
//        }
//    }
}