//
// Created by dsporykhin on 13.02.21.
//
#include <Arduino.h>
#include <ArduinoOTA.h>
#include "WiFiController.h"
#include "MqttClient.h"
#include "Switcher.h"
#include "DallasTemperature.h"

#include <ESP8266WiFi.h>


SettingsManager *settingsManager;
WiFiController *wiFiController;
MqttClient *mqtt;
Switcher *switcher;
OneWire *oneWire;
DallasTemperature *dallasTemperature;
long lastTempRead = 0;

int sensors_count;
uint8_t *sensorAddr = (uint8_t *) malloc(200);

//bool a0Started;
//long startedAt;
//long lastWorkA;

void setup() {
 //   pinMode(13, OUTPUT);
#ifdef CON_DEBUG
    Serial.begin(115200);
    Serial.println("---");

    LOGGER.info("Started UART at 921600");
#endif

    LOGGER.info("Starting...");

    settingsManager = new SettingsManager();

    wiFiController = new WiFiController(settingsManager);

    mqtt = new MqttClient(settingsManager->getSettings());

    ArduinoOTA.begin(true);

    LOGGER.info("start device");

    switcher = new Switcher(settingsManager->getSettings(), mqtt);

    LOGGER.info("starting DS18D20...");

    oneWire = new OneWire(ONE_WIRE_PIN);
    dallasTemperature = new DallasTemperature(oneWire);

    dallasTemperature->begin();
    sensors_count = dallasTemperature->getDS18Count();
    for (int termo_index = 0; termo_index < sensors_count; termo_index++){
        LOGGER.info("   temperature sensor " + String(termo_index) + " found");
        dallasTemperature->getAddress(&sensorAddr[termo_index * 10], termo_index);
    }
    dallasTemperature->setResolution(12);

    if (!sensors_count){
        LOGGER.info("   temperature sensor NOT found");
    }

    LOGGER.info("all done");
}


void loop() {
    ArduinoOTA.handle();
    mqtt->dispatch();
    switcher->dispatch();
    wiFiController->checkConnection();

    if (sensors_count && (lastTempRead == 0 || ((millis() - lastTempRead) > 500))) {
        lastTempRead = millis();
        for (int index = 0; index < sensors_count; index++){
            dallasTemperature->requestTemperatures();
            float tempr = dallasTemperature->getTempC(&sensorAddr[index * 10]);
            if (tempr != -127) {
               mqtt->sendToCustomTopic("sensor" + String(index), String(tempr));
            }
            LOGGER.info(String(index) + " = " + String(tempr));
        }
    }
}