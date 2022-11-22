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
#include <esp_task_wdt.h>
#include <lib/math/Intervals.h>
#include "Logger.h"
#include "SettingsManager.h"
#include "ArduinoOTA.h"
#include "WiFiController.h"
#include "MqttClient.h"
#include "MqttCommandsReceiver.h"
#include "Display.h"

SettingsManager *settingsManager;
WiFiController *wiFiController;
MqttClient *mqtt;
MqttCommandsReceiver *mqttCommandsReceiver;
SensorController *sensorController;
HeaterController *heaterController;
Display *display;
DisplayButtonController *displayButtonController;

long lastTimeDisplayed;

void towDeviceInfo(const char *msg) {
    LOGGER.info(msg);
    display->printStatus(msg);

}

long last_wdt_reset = 0;

void reset_wdt() {
    if (millis() - last_wdt_reset > 3000) {
        last_wdt_reset = millis();
        esp_task_wdt_reset();
    }
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
    esp_task_wdt_init(20, true); //enable panic so ESP32 restarts
    esp_task_wdt_add(NULL); //add current thread to WDT watch

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

    reset_wdt();
    towDeviceInfo("starting WiFi...");
    wiFiController = new WiFiController(settingsManager);

    towDeviceInfo("starting HeaterController...");
    wiFiController->setHeaterController(heaterController);

    towDeviceInfo("starting MqTT...");
    mqtt = new MqttClient(settingsManager->getSettings());

    towDeviceInfo("starting MqTT controller...");
    mqttCommandsReceiver = new MqttCommandsReceiver(settingsManager->getSettings(), mqtt, heaterController);
    mqttCommandsReceiver->display = display;

    reset_wdt();
    towDeviceInfo("starting OTA");
    ArduinoOTA.onStart([]() {
        Adafruit_ST7789 *tft = display->getTft();
        tft->fillScreen(COLOR_BACKGROUND);
        tft->setCursor(10, 60);
        tft->print("OTA Updating...");
        LOGGER.info("OTA begins...");
    });
    ArduinoOTA.onProgress([](unsigned int progress, unsigned int total) {
        Adafruit_ST7789 *tft = display->getTft();

        tft->fillRect(0, 120, 318, 100, COLOR_BACKGROUND);
        tft->setCursor(10, 160);
        tft->print("Loading " + String(progress) + " of " + String(total));
        LOGGER.info("OTA loading " + String(progress) + " of " + String(total));
        reset_wdt();
    });
    ArduinoOTA.onError([](ota_error_t error) {
        display->setScreenIndex(0);
        display->drawScreen();
    });
    ArduinoOTA.begin();

    towDeviceInfo("start displayBtn controller");
    displayButtonController = new DisplayButtonController(heaterController, display);

    towDeviceInfo("all done");

    LOGGER.info("lib has " + String(settingsManager->getNavigator()->getParamDescriptorCounter()));
    delay(500);

    reset_wdt();
    display->setScreenIndex(0);

    lastTimeDisplayed = 0;
}

long last_report_to_mqtt = 0;

void loop() {
//    char raw_csv_buffer[1280]; // FIXME
    ArduinoOTA.handle();
    mqtt->dispatch();
    wiFiController->checkConnection();
//    LOGGER.handle();
    displayButtonController->handle();

    if (((heaterController->handle()) && !heaterController->isModelling())
        || (heaterController->isModelling() && ((millis() - lastTimeDisplayed) > 3000))) {

        reset_wdt();

        lastTimeDisplayed = millis();
        TelemetryDataRecord *telemetryDataRecord = heaterController->getTelemetryRecord();
        display->updateInfo(telemetryDataRecord);
        if ((millis() - last_report_to_mqtt) >= settingsManager->getSettings()->send_data_to_mqtt_interval_ms) {
            last_report_to_mqtt = millis();

            mqtt->sendToCustomTopic(settingsManager->getSettings()->deviceStateOutgoingTopicPrefix,
                                    heaterController->isOxygenDoorOpenedForATime() ? "ON" : "OFF");
        }
    }
}
