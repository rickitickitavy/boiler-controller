//
// Created by dsporykhin on 13.02.21.
//
#include <Arduino.h>
#include <ArduinoOTA.h>
#include "WiFiController.h"
#include "MqttClient.h"
#include "SwitcherX4.h"
#include "DallasTemperature.h"
#include "Converter.h"
#include "SensorController.h"
#include <ESP8266WiFi.h>
#include <Wire.h>
#include "drivers/PwmPCA9685Driver.h"
#include "HeaterController.h"

SettingsManager *settingsManager;
WiFiController *wiFiController;
MqttClient *mqtt;
SwitcherX4 *switcher;
SensorController *sensorController;
PwmPCA9685Driver *pwmDriver;
HeaterController *heaterController;
bool pwm_ready;
long lastTempRead = 0;


void setup() {
#ifdef CON_DEBUG
    Serial.begin(115200);
    Serial.println("---");

    LOGGER.info("Started UART at 921600");
#endif

    LOGGER.info("Starting...");

    settingsManager = new SettingsManager();

    sensorController = new SensorController(ONE_WIRE_PIN, settingsManager);
    settingsManager->getNavigator()->setSensorList(sensorController->buildSensorsList());

    Wire.begin(4, 5);
    Wire.setClock(400000);
    pwmDriver = new PwmPCA9685Driver(settingsManager->getSettings()->pwm_controller_address);
    pwm_ready = pwmDriver->detected;
    if (pwmDriver->detected) {
        LOGGER.info("PWM detected");
        pwmDriver->setPwmFrequency(100);
        heaterController = new HeaterController(settingsManager->getSettings(), pwmDriver, sensorController);
    } else
        LOGGER.info("PWM failed");

    settingsManager->getNavigator()->addParamDescriptor(
            new ParamDescriptor("control>servo>pwm_controller_initialized", BOOLEAN, 0,
                                15,
                                (void *) &pwm_ready,
                                (void *) nullptr));

    LOGGER.info("PTR = " +  String(((int)((void *) &pwm_ready))));

    wiFiController = new WiFiController(settingsManager);

    mqtt = new MqttClient(settingsManager->getSettings());

    ArduinoOTA.begin(true);

    LOGGER.info("start device");

    switcher = new SwitcherX4(settingsManager->getSettings(), mqtt);

    LOGGER.info("starting DS18D20...");

    LOGGER.info("all done");

}

void loop() {
    ArduinoOTA.handle();
    mqtt->dispatch();
    wiFiController->checkConnection();

    sensorController->handle();

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
// 1073674992