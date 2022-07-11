//
// Created by dsporykhin on 13.02.21.
//
#include <Wire.h>
#include <HardwareSerial.h>
#include <SPIFFS.h>
#include <SD.h>
#include <lib/math/Sma.h>
#include "Logger.h"
#include "SettingsManager.h"
#include "ArduinoOTA.h"
#include "WiFiController.h"
#include "MqttClient.h"
#include "SwitcherX4.h"
#include "SensorController.h"
#include "HeaterController.h"

SettingsManager *settingsManager;
WiFiController *wiFiController;
MqttClient *mqtt;
SwitcherX4 *switcher;
SensorController *sensorController;
HeaterController *heaterController;
bool pwm_ready;
long lastTempRead = 0;
bool sd_present;

void setup() {
//    byte i;
//    byte addr[8];

#ifdef CON_DEBUG
    Serial.begin(921600);
    Serial.println("---");

    LOGGER.info("Started UART at 115200");
#endif
    LOGGER.info("Starting...");

    settingsManager = new SettingsManager();

    LOGGER.info("starting DS18D20...");
    sensorController = new SensorController(ONE_WIRE_PIN, settingsManager);
    settingsManager->getNavigator()->setSensorList(sensorController->buildSensorsList());

    LOGGER.error("Starting I2C");
    Wire.begin(4, 5);
    Wire.setClock(400000);

    LOGGER.error("Starting heater controller...");
    heaterController = new HeaterController(settingsManager->getSettings(), sensorController, settingsManager->getNavigator());

    LOGGER.error("Mounting SD...");
    if (!SPIFFS.begin(false)) {
        LOGGER.error(" Mount Failed");
    }

    wiFiController = new WiFiController(settingsManager);
    wiFiController->setHeaterController(heaterController);

    mqtt = new MqttClient(settingsManager->getSettings());

    LOGGER.info("starting OTA");
    ArduinoOTA.begin();

    LOGGER.info("start device");
    switcher = new SwitcherX4(settingsManager->getSettings(), mqtt);


    LOGGER.info("all done");

    LOGGER.info("lib has " + String(settingsManager->getNavigator()->getParamDescriptorCounter()));

}

int cycle_index = 0;

void loop() {
    ArduinoOTA.handle();
    mqtt->dispatch();
    wiFiController->checkConnection();
    LOGGER.handle();

    heaterController->handle();

//    if (sensorController->data_ready) {
//        sensorController->data_ready = false;
//        for (int index = 0; index < MAX_SENSORS_COUNT; index++) {
//            if (sensorController->sensor_data[index].data_ready) {
//                sensorController->sensor_data[index].data_ready = false;
//                mqtt->sendToCustomTopic("sensor" + String(index), String(sensorController->sensor_data[index].value));
//                LOGGER.info(String(index) + " = " + String(sensorController->sensor_data[index].value));
//            }
//        }
//    }

}
