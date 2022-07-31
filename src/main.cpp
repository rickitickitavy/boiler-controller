//
// Created by dsporykhin on 13.02.21.
//
#include <Wire.h>
#include <HardwareSerial.h>
#include <SPIFFS.h>
#include <SD.h>
#include "Logger.h"
#include "SettingsManager.h"
#include "ArduinoOTA.h"
#include "WiFiController.h"
#include "MqttClient.h"
#include "SwitcherX4.h"
#include "Display.h"

SettingsManager *settingsManager;
WiFiController *wiFiController;
MqttClient *mqtt;
SwitcherX4 *switcher;
SensorController *sensorController;
HeaterController *heaterController;
Display *display;
long lastTimeDisplayed;

void towDeviceInfo(const char *msg){
    LOGGER.info(msg);
    display->printStatus(msg);

}

void setup() {
//    byte i;
//    byte addr[8];

#ifdef CON_DEBUG
    Serial.begin(921600);
    Serial.println("---");

    LOGGER.info("Started UART at 115200");
#endif
    display = new Display();

    towDeviceInfo("Starting...");
//    FlowSensor *flowSensor = new FlowSensor(39);
//
//    for (int i =0; i< 200; i++){
//        Serial.println(String(i) + " sens = " + String(flowSensor->readAndReset()));
//        delay(500);
//    }

//
//    pinMode(22, OUTPUT);
//    while (true) {
//        long next = millis() + 3;
//
//        for (int i = 500; i > 45;) {
//            if (millis() > next) {
//                i--;
//                next = millis() + 2;
//            }
//
//            digitalWrite(22, HIGH);
//            delayMicroseconds(5);
//            digitalWrite(22, LOW);
//            delayMicroseconds(i);
//        }
//
//        for (int i = 0; i < 10000; i++) {
//            digitalWrite(22, HIGH);
//            delayMicroseconds(5);
//            digitalWrite(22, LOW);
//            delayMicroseconds(45);
//        }
//
//        next = millis() + 3;
//        for (int i = 45; i < 500;) {
//            if (millis() > next) {
//                i++;
//                next = millis() + 2;
//            }
//
//            digitalWrite(22, HIGH);
//            delayMicroseconds(5);
//            digitalWrite(22, LOW);
//            delayMicroseconds(i);
//        }
//
//    }
    settingsManager = new SettingsManager();

    towDeviceInfo("starting DS18D20...");
    sensorController = new SensorController(ONE_WIRE_PIN, settingsManager);
    settingsManager->getNavigator()->setSensorList(sensorController->buildSensorsList());

    towDeviceInfo("Starting I2C");
    Wire.begin(4, 5);
    Wire.setClock(400000);

    towDeviceInfo("Starting heater controller...");
    heaterController = new HeaterController(settingsManager->getSettings(), sensorController,
                                            settingsManager->getNavigator());

    towDeviceInfo("Mounting SD...");
    if (!SPIFFS.begin(false)) {
        LOGGER.error(" Mount Failed");
    }

    towDeviceInfo("starting WiFi...");
    wiFiController = new WiFiController(settingsManager);

    towDeviceInfo("starting HeaterController...");
    wiFiController->setHeaterController(heaterController);

    towDeviceInfo("starting MqTT...");
    mqtt = new MqttClient(settingsManager->getSettings());

    towDeviceInfo("starting OTA");
    ArduinoOTA.begin();

    towDeviceInfo("start device");
    switcher = new SwitcherX4(settingsManager->getSettings(), mqtt);


    towDeviceInfo("all done");

    LOGGER.info("lib has " + String(settingsManager->getNavigator()->getParamDescriptorCounter()));
    delay(500);
    display->setScreenIndex(0);

    lastTimeDisplayed = 0;
}

int cycle_index = 0;

void loop() {
    ArduinoOTA.handle();
    mqtt->dispatch();
    wiFiController->checkConnection();
    LOGGER.handle();

    if (((heaterController->handle()) && !heaterController->isModelling())
    || (heaterController->isModelling() && ((millis() - lastTimeDisplayed) > 3000))){
        lastTimeDisplayed = millis();
        TelemetryDataRecord *telemetryDataRecord = heaterController->getTelemetryRecord();
        char *mode;
        switch (telemetryDataRecord->heaterMode){
            case STAND_BY :
                mode = "Mode: STANDBY";
                break;
            case WARMING :
                mode = "Mode: WARMING";
                break;
            case PID :
                mode = "Mode: NORMAL BURNING";
                break;
            case OVERHEATED :
                mode = "Mode: OVERHEATED";
                break;
            case CRITICAL :
                mode = "Mode: CRITICAL";
                break;
            case FINAL_COOLING :
                mode = "Mode: FINAL_COOLING";
                break;
        }
        display->setScreen0Parameter(0, mode, telemetryDataRecord->main_door_opened ? "OPENED" : "CLOSED");
        display->setScreen0Parameter(1, "Core t (°C)", String(telemetryDataRecord->core_temp_sma).c_str());
        display->setScreen0Parameter(2, "Core pwr (Watt)", String((int)telemetryDataRecord->core_EMA_power).c_str());
        display->setScreen0Parameter(3, "Core input (°C)", String(telemetryDataRecord->input_temp_sma).c_str());
        display->setScreen0Parameter(4, "Core output (°C)", String(telemetryDataRecord->output_temp_sma).c_str());
        display->setScreen0Parameter(5, "Core flow(l/min)", String(telemetryDataRecord->core_flow).c_str());
        display->setScreen0Parameter(6, "Accum top(°C)", String(telemetryDataRecord->accumulator_top_temp_sma).c_str());
        display->setScreen0Parameter(7, "Accum midHi(°C)", String(telemetryDataRecord->accumulator_higher_temp_sma).c_str());
        display->setScreen0Parameter(8, "Accum midLo(°C)", String(telemetryDataRecord->accumulator_lower_temp_sma).c_str());
        display->setScreen0Parameter(9, "Accum bottom(°C)", String(telemetryDataRecord->accumulator_bottom_temp_sma).c_str());

        display->drawScreen();
    }

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
