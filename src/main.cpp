//
// Created by dsporykhin on 13.02.21.
//
#include <Wire.h>
#include <HardwareSerial.h>
#include <SPIFFS.h>
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


void setup() {
//    byte i;
//    byte addr[8];

#ifdef CON_DEBUG
    Serial.begin(115200);
    Serial.println("---");

    LOGGER.info("Started UART at 115200");
#endif
    LOGGER.info("Starting...");

//    delay (1000);
//
//    OneWire *ow = new OneWire(14);
//    ow->reset_search();
//    delay(250);
//
//    if (!ow->search(addr, true)) {
//        Serial.println(" No more addresses.");
//        Serial.println();
//    } else {
//        Serial.println(" Found devices.");
//        for (i = 0; i < 8; i++) {
//            Serial.write(' ');
//            Serial.print(addr[i], HEX);
//        }
//    }
//    ow->reset_search();
//    delay(250);
//    ow->depower();
//
//    delay (1000);
//    ESP.restart();
//
    settingsManager = new SettingsManager();

    LOGGER.info("starting DS18D20...");
    sensorController = new SensorController(ONE_WIRE_PIN, settingsManager);
    settingsManager->getNavigator()->setSensorList(sensorController->buildSensorsList());

    Wire.begin(4, 5);
    Wire.setClock(400000);

    heaterController = new HeaterController(settingsManager->getSettings(), sensorController, settingsManager->getNavigator());

    if (!SPIFFS.begin(false)) {
        LOGGER.error(" Mount Failed");
    }

    wiFiController = new WiFiController(settingsManager);

    mqtt = new MqttClient(settingsManager->getSettings());

    LOGGER.info("starting OTA");
    ArduinoOTA.begin();

    LOGGER.info("start device");
    switcher = new SwitcherX4(settingsManager->getSettings(), mqtt);


    LOGGER.info("all done");
}

void loop() {
    ArduinoOTA.handle();
    mqtt->dispatch();
    wiFiController->checkConnection();

//    sensorController->handle();

    if (sensorController->data_ready) {
        sensorController->data_ready = false;
        for (int index = 0; index < MAX_SENSORS_COUNT; index++) {
            if (sensorController->sensor_data[index].data_ready) {
                sensorController->sensor_data[index].data_ready = false;
                mqtt->sendToCustomTopic("sensor" + String(index), String(sensorController->sensor_data[index].value));
                LOGGER.info(String(index) + " = " + String(sensorController->sensor_data[index].value));
            }
        }
    }

}
