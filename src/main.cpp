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
bool termoSensorExists;
long lastTempRead = 0;
uint8_t *sensorAddr = (uint8_t *) malloc(10);

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
    termoSensorExists = dallasTemperature->getDS18Count() > 0;

    if (termoSensorExists) {
        LOGGER.info("   temperature sensor found");
        dallasTemperature->getAddress(sensorAddr, 0);
        dallasTemperature->setResolution(12);
    } else {
        LOGGER.info("   temperature sensor NOT found");
    }

    LOGGER.info("all done");
}


void loop() {
    ArduinoOTA.handle();
    mqtt->dispatch();
    switcher->dispatch();
    wiFiController->checkConnection();

    if (termoSensorExists && (lastTempRead == 0 || ((millis() - lastTempRead) > 60000))) {
        lastTempRead = millis();
        dallasTemperature->requestTemperatures();
        float tempr = dallasTemperature->getTempC(sensorAddr);
        if (tempr != -127) {
            mqtt->sendToCustomTopic("sensor0", String(tempr));
        }
        LOGGER.info(String(tempr));
    }

}