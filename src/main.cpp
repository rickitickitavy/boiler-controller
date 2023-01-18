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
#include "Display.h"

SettingsManager *settingsManager;
WiFiController *wiFiController;
SensorController *sensorController;
HeaterController *heaterController;
Display *display;
DisplayButtonController *displayButtonController;

long lastTimeDisplayed;

void twoDeviceInfo(const char *msg) {
    LOGGER.info(msg);
    display->printStatus(msg);

}

long last_wdt_reset = 0;

void reset_wdt() {
    if (millis() - last_wdt_reset > 3000) {
        last_wdt_reset = millis();
        esp_task_wdt_reset();
        digitalWrite(5, LOW);
        delay(1);
        digitalWrite(5, HIGH);
    }
}

void setup() {

#ifdef CON_DEBUG
    Serial.begin(921600);
    Serial.println("---");

    LOGGER.info("Started UART at 115200");
#endif
    pinMode(5, OUTPUT);
    reset_wdt();

    display = new Display();


    esp_task_wdt_init(25, true); //enable panic so ESP32 restarts
    esp_task_wdt_add(NULL); //add current thread to WDT watch

    twoDeviceInfo("Starting...");
    settingsManager = new SettingsManager();

    twoDeviceInfo("starting DS18D20...");
    sensorController = new SensorController(ONE_WIRE_PIN, settingsManager);
    settingsManager->getNavigator()->setSensorList(sensorController->buildSensorsList());

    twoDeviceInfo("Starting I2C");
    Wire.begin(21, 22);
    Wire.setClock(400000);

    twoDeviceInfo("Starting heater controller...");
    heaterController = new HeaterController(settingsManager->getSettings(), sensorController,
                                            settingsManager->getNavigator());

//    display->getTft()->drawImage(display->init_fire_off, 10, 10, 74, 77);
//    display->getTft()->drawImage(display->init_fire_on, 10, 100, 74, 77);
//    display->getTft()->drawImage(display->pumps_off, 100, 10, 86, 62);
//    display->getTft()->drawImage(display->pumps_on, 100, 100, 86, 62);


    LOGGER.info("Mounting internal flash...");
    if (!SPIFFS.begin(false, "/spiffs", 5)) {
        LOGGER.error(" Mount Failed");
    } else
    LOGGER.error("   mounted");


    reset_wdt();
    twoDeviceInfo("starting WiFi...");
    wiFiController = new WiFiController(settingsManager);

    twoDeviceInfo("starting HeaterController...");
    wiFiController->setHeaterController(heaterController);

    reset_wdt();
    twoDeviceInfo("starting OTA");
    ArduinoOTA.onStart([]() {
        ILI9488 *tft = display->getTft();
        tft->fillScreen(COLOR_BACKGROUND);
        tft->setCursor(10, 60);
        tft->print("OTA Updating...");
        LOGGER.info("OTA begins...");
    });
    ArduinoOTA.onProgress([](unsigned int progress, unsigned int total) {
        ILI9488 *tft = display->getTft();

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

    twoDeviceInfo("start displayBtn controller");
    displayButtonController = new DisplayButtonController(heaterController, display);

    LOGGER.info("lib has " + String(settingsManager->getNavigator()->getParamDescriptorCounter()));
    delay(500);

    reset_wdt();
    twoDeviceInfo("all done");

    display->setScreenIndex(0);
    lastTimeDisplayed = 0;
    LOGGER.info("display->setScreenIndex(0) done ");
    delay(500);
}

void loop() {
    ArduinoOTA.handle();
    wiFiController->checkConnection();
    displayButtonController->handle();
    reset_wdt();

    if (((heaterController->handle()) && !heaterController->isModelling())
        || (heaterController->isModelling() && ((millis() - lastTimeDisplayed) > 3000))) {


        lastTimeDisplayed = millis();
        TelemetryDataRecord *telemetryDataRecord = heaterController->getTelemetryRecord();
//
//        LOGGER.info("before display->updateInfo(telemetryDataRecord)");
//        Serial.flush();
//
        display->updateInfo(telemetryDataRecord);
//
//        LOGGER.info("after display->updateInfo(telemetryDataRecord)");
//        Serial.flush();
    }
}
