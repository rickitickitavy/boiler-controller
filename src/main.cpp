//
// Created by dsporykhin on 13.02.21.
//
#include <Wire.h>
#include <HardwareSerial.h>
#include <SPIFFS.h>
#include <SD.h>
#include <DisplayButtonController.h>
#include <lib/adafruit/Fonts/FreeSerif12pt7b.h>
#include <lib/adafruit/Fonts/FreeSans12pt7b.h>
#include <lib/adafruit/Fonts/FreeMono12pt7b.h>
#include <lib/adafruit/Fonts/TomThumb.h>
#include <lib/adafruit/Fonts/Picopixel.h>
#include <lib/adafruit/Fonts/Org_01.h>
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
DisplayButtonController *displayButtonController;

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

//    TelemetryDataRecord temp;
//    temp.heaterMode = STAND_BY;
//    temp.internal_temp = 35.45;
//    temp.main_door_opened = false;
//    temp.accumulator_bottom_temp_sma = 40.88;
//    temp.accumulator_lower_temp_sma = 50.88;
//    temp.accumulator_higher_temp_sma = 60.88;
//    temp.accumulator_top_temp_sma = 70.88;
//    temp.core_flow = 32.65;
//    temp.output_temp_sma = 80.88;
//    temp.input_temp_sma = 55.88;
//    temp.core_temp_sma = 90.88;
//    temp.core_EMA_power = 88888.99;
//    temp.upper_door_position = 68.88;
//    temp.oxygen_door_position = 88.88;
//    temp.smoke_door_position = 98.88;
//    temp.pid_p = 888.888;
//    temp.pid_i = 999.888;
//    temp.pid_d = 555.888;
//    temp.pump_1_state = 0;
//    temp.pump_2_state = 1;
//
//    display->setScreenIndex(0);
////    delay(1000);
//
//    display->updateInfo(&temp);
//    delay(1000);
//
//    temp.heaterMode = WARMING;
//    display->updateInfo(&temp);
//    delay(1000);
//
//    temp.heaterMode = PID;
//    temp.time_to_close_oxygen_door_in_stanby_mode = millis() + 660000;
//    display->updateInfo(&temp);
//    delay(1000);
//
//    temp.heaterMode = OVERHEATED;
//    temp.main_door_opened = true;
//    display->updateInfo(&temp);
//    delay(1000);
//
//    temp.heaterMode = CRITICAL;
//    temp.main_door_opened = false;
//    display->updateInfo(&temp);
//    delay(1000);
//
//    temp.pump_1_state = true;
//    display->updateInfo(&temp);
//    delay(1000);
//
//    temp.pump_1_state = true;
//    display->updateInfo(&temp);
//    delay(1000);
//
//    temp.pump_2_state = false;
//    display->updateInfo(&temp);
//    delay(1000);
//
////    Adafruit_ST7789 *tft = display->getTft();
//    delay(10000);
//
    towDeviceInfo("Starting...");
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

    towDeviceInfo("start displayBtn controller");
    displayButtonController = new DisplayButtonController(heaterController, display);

    towDeviceInfo("all done");

    LOGGER.info("lib has " + String(settingsManager->getNavigator()->getParamDescriptorCounter()));
    delay(500);

    display->setScreenIndex(0);

    lastTimeDisplayed = 0;
}

void loop() {
    ArduinoOTA.handle();
    mqtt->dispatch();
    wiFiController->checkConnection();
    LOGGER.handle();
    displayButtonController->handle();

    if (((heaterController->handle()) && !heaterController->isModelling())
    || (heaterController->isModelling() && ((millis() - lastTimeDisplayed) > 3000))){
        lastTimeDisplayed = millis();
        TelemetryDataRecord *telemetryDataRecord = heaterController->getTelemetryRecord();
        display->updateInfo(telemetryDataRecord);
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
