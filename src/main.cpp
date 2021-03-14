//
// Created by dsporykhin on 13.02.21.
//
#include <Arduino.h>
#include <ArduinoOTA.h>
#include "WiFiController.h"
#include "MqttClient.h"
#include "Switcher.h"

#include <ESP8266WiFi.h>


SettingsManager *settingsManager;
WiFiController *wiFiController;
MqttClient *mqtt;
Switcher *switcher;

//bool a0Started;
//long startedAt;
//long lastWorkA;

void setup() {
    pinMode(13, OUTPUT);
#ifdef CON_DEBUG
    Serial.begin(74880);
    Serial.println("---");

    LOGGER.info("Started UART at 921600");
#endif

//    a0Started = false;
//    WiFi.begin("TP-LINK_B1C6", "15121820");
//    startedAt = millis();
//    while (!WiFi.isConnected() && (millis() - startedAt < 10000)) {
//        delay(20);
//    }
//
//    if (!WiFi.isConnected()) {
//        LOGGER.info("Not connected: rebooting...");
//        delay(500);
//        ESP.restart();
//    }
//    LOGGER.info("Connected.");
//    LOGGER.info(WiFi.localIP().toString());
//    lastWorkA = millis();
//    pinMode(A0, INPUT);

    LOGGER.info("Starting...");

    ArduinoOTA.begin(true);

    settingsManager = new SettingsManager();

    wiFiController = new WiFiController(settingsManager);

    mqtt = new MqttClient(settingsManager->getSettings());

    LOGGER.info("start device");

    switcher = new Switcher(settingsManager->getSettings(), mqtt);

    LOGGER.info("all done");
}


void loop() {
//    digitalWrite(13, LOW);
//    delay(200);
//    digitalWrite(13, HIGH);
//    delay(200);

    ArduinoOTA.handle();
    mqtt->dispatch();
    switcher->dispatch();
//    if (millis() > 19000) {
//        if (!a0Started) {
//            a0Started = true;
//            LOGGER.info("A0 started");
//        }
//            if (analogRead(A0) > 0){
//             delay(10);
//            }
//    }
}