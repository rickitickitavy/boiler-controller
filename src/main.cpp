//
// Created by dsporykhin on 13.02.21.
//
#include <Wire.h>
#include <HardwareSerial.h>
#include <LittleFS.h>
#include <esp_task_wdt.h>
#include "Logger.h"
#include "SettingsManager.h"
#include "ArduinoOTA.h"
#include "WiFiController.h"
#include "TouchDisplayController.h"

SettingsManager *settingsManager;
WiFiController *wiFiController;
SensorController *sensorController;
HeaterController *heaterController;
TouchDisplayController *touchDisplayController;
//TouchController *displayButtonController;

long lastTimeDisplayed;

void twoDeviceInfo(const char *msg) {
    LOGGER.info(msg);
    touchDisplayController->printStatus(msg);

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

long setup_finished_at = 0;
bool screen_setup_finished = false;

void setup() {

#ifdef CON_DEBUG
    Serial.begin(921600);
    Serial.println("---");

    LOGGER.info("Started UART at 115200");
#endif
    pinMode(5, OUTPUT);
    reset_wdt();

    touchDisplayController = new TouchDisplayController();

    esp_task_wdt_init(25, true); //enable panic so ESP32 restarts
    esp_task_wdt_add(NULL); //add current thread to WDT watch

    twoDeviceInfo("Starting...");
    settingsManager = new SettingsManager();

    twoDeviceInfo("starting DS18D20...");
    delay(500);
    sensorController = new SensorController(ONE_WIRE_PIN, ONE_WIRE_PIN_2, settingsManager);
    settingsManager->getNavigator()->setSensorList(sensorController->buildSensorsList());

    twoDeviceInfo("Starting I2C");
    Wire.begin(21, 22);
    Wire.setClock(400000);

    twoDeviceInfo("Starting heater controller...");
    heaterController = new HeaterController(settingsManager->getSettings(), sensorController,
                                            settingsManager->getNavigator());

    touchDisplayController->setHeaterController(heaterController);

    LOGGER.info("Mounting internal flash...");
    if (!LittleFS.begin(false)) {
        LOGGER.error(" Mount Failed");
    } else
        LOGGER.info("   mounted");

    reset_wdt();
    twoDeviceInfo("starting WiFi...");
    wiFiController = new WiFiController(settingsManager);
    wiFiController->getWebServerController()->setTouchDisplayController(touchDisplayController);

    twoDeviceInfo("starting HeaterController...");
    wiFiController->setHeaterController(heaterController);

    reset_wdt();
    twoDeviceInfo("starting OTA");
    ArduinoOTA.onStart([]() {
        ILI9488 *tft = touchDisplayController->getTft();
        tft->fillScreen(COLOR_BACKGROUND);
        tft->setCursor(10, 60);
        tft->print("OTA Updating...");
        LOGGER.info("OTA begins...");
    });
    ArduinoOTA.onProgress([](unsigned int progress, unsigned int total) {
        ILI9488 *tft = touchDisplayController->getTft();

        tft->fillRect(0, 120, 318, 100, COLOR_BACKGROUND);
        tft->setCursor(10, 160);
        tft->print("Loading " + String(progress) + " of " + String(total));
        LOGGER.info("OTA loading " + String(progress) + " of " + String(total));
        reset_wdt();
    });
    ArduinoOTA.onError([](ota_error_t error) {
        touchDisplayController->setScreenIndex(0);
        touchDisplayController->drawScreen();
    });
    ArduinoOTA.begin();

    twoDeviceInfo("start displayBtn controller");

    LOGGER.info("lib has " + String(settingsManager->getNavigator()->getParamDescriptorCounter()));
    delay(500);

    reset_wdt();
    twoDeviceInfo("all done");

    touchDisplayController->getTft()->fillScreen(COLOR_BACKGROUND);
    touchDisplayController->getTft()->setCursor(10, 160);
    twoDeviceInfo("WAITING FOR OTA FOR 6 SECONDS...");
    setup_finished_at = millis();
}

void loop() {
    ArduinoOTA.handle();
    wiFiController->checkConnection();
    reset_wdt();

    // give first 3 seconds work OTA only
    if ((millis() - setup_finished_at) < 6000)
        return;
    else if (!screen_setup_finished){
        twoDeviceInfo("DONE.");
        delay(500);
        touchDisplayController->setScreenIndex(0);
        lastTimeDisplayed = 0;
        LOGGER.info("display->setScreenIndex(0) done ");
        screen_setup_finished = true;
    }

    touchDisplayController->handle();
    sensorController->readNextBlockOfSensors();

    if (((heaterController->handle()) && !heaterController->isModelling())
        || (heaterController->isModelling() && ((millis() - lastTimeDisplayed) > 3000))) {


        lastTimeDisplayed = millis();
        TelemetryDataRecord *telemetryDataRecord = heaterController->getTelemetryRecord();

        touchDisplayController->setTimeToLiveValue(millis());

        touchDisplayController->updateInfo(telemetryDataRecord);
    }
    if (touchDisplayController->isDirty())
        touchDisplayController->drawScreen();

}
