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
#include "Gauge.h"

SettingsManager *settingsManager;
WiFiController *wiFiController;
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

    esp_task_wdt_init(20, true); //enable panic so ESP32 restarts
    esp_task_wdt_add(NULL); //add current thread to WDT watch

    towDeviceInfo("Starting...");
    settingsManager = new SettingsManager();

    towDeviceInfo("starting DS18D20...");
    sensorController = new SensorController(ONE_WIRE_PIN, settingsManager);
    settingsManager->getNavigator()->setSensorList(sensorController->buildSensorsList());

    towDeviceInfo("Starting I2C");
    Wire.begin(21, 22);
    Wire.setClock(400000);

    towDeviceInfo("Starting heater controller...");
    heaterController = new HeaterController(settingsManager->getSettings(), sensorController,
                                            settingsManager->getNavigator());

    towDeviceInfo("Mounting internal flash...");
    if (!SPIFFS.begin(false)) {
        LOGGER.error(" Mount Failed");
    }

//    reset_wdt();
//    towDeviceInfo("starting WiFi...");
//    wiFiController = new WiFiController(settingsManager);
//
//    towDeviceInfo("starting HeaterController...");
//    wiFiController->setHeaterController(heaterController);
//
    reset_wdt();
//    towDeviceInfo("starting OTA");
//    ArduinoOTA.onStart([]() {
//        ILI9488 *tft = display->getTft();
//        tft->fillScreen(COLOR_BACKGROUND);
//        tft->setCursor(10, 60);
//        tft->print("OTA Updating...");
//        LOGGER.info("OTA begins...");
//    });
//    ArduinoOTA.onProgress([](unsigned int progress, unsigned int total) {
//        ILI9488 *tft = display->getTft();
//
//        tft->fillRect(0, 120, 318, 100, COLOR_BACKGROUND);
//        tft->setCursor(10, 160);
//        tft->print("Loading " + String(progress) + " of " + String(total));
//        LOGGER.info("OTA loading " + String(progress) + " of " + String(total));
//        reset_wdt();
//    });
//    ArduinoOTA.onError([](ota_error_t error) {
//        display->setScreenIndex(0);
//        display->drawScreen();
//    });
//    ArduinoOTA.begin();

    towDeviceInfo("start displayBtn controller");
    displayButtonController = new DisplayButtonController(heaterController, display);

    towDeviceInfo("all done");

    LOGGER.info("lib has " + String(settingsManager->getNavigator()->getParamDescriptorCounter()));
    delay(500);

    reset_wdt();
    display->setScreenIndex(0);

    lastTimeDisplayed = 0;

//    Gauge *gauge = new Gauge(display->getTft(), 3, 3, 100, 100, ILI9488_BLACK,
//                             0xffffff, NULL, 0, -100, 100, new ColorPart(ILI9488_BLUE, -10,
//                                                                         new ColorPart(ILI9488_GREEN, 50,
//                                                                                       new ColorPart(
//                                                                                               ILI9488_RED,
//                                                                                               0, NULL))));
//    gauge->init();
//    gauge->draw(true);

}

void loop() {
//    ArduinoOTA.handle();
//    wiFiController->checkConnection();
    displayButtonController->handle();

    reset_wdt();

    if (((heaterController->handle()) && !heaterController->isModelling())
        || (heaterController->isModelling() && ((millis() - lastTimeDisplayed) > 3000))) {


        lastTimeDisplayed = millis();
        TelemetryDataRecord *telemetryDataRecord = heaterController->getTelemetryRecord();
//        display->updateInfo(telemetryDataRecord);
    }
}
