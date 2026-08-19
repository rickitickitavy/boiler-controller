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
#include "MqttController.h"
#include "TouchDisplayController.h"

SettingsManager *settingsManager;
WiFiController *wiFiController;
MqttController *mqttController;
SensorController *sensorController;
HeaterController *heaterController;
TouchDisplayController *touchDisplayController;
//TouchController *displayButtonController;

long lastTimeDisplayed;
long lastHeapLogMs = 0;

void twoDeviceInfo(const char *msg) {
    LOGGER.info(msg);
    touchDisplayController->printStatus(msg);

}

long last_wdt_reset = 0;

void reset_wdt() {
    if (millis() - last_wdt_reset > 3000) {
        last_wdt_reset = millis();
        esp_task_wdt_reset();
        digitalWrite(EXTERNAL_WDT_PIN, LOW);
        delay(1);
        digitalWrite(EXTERNAL_WDT_PIN, HIGH);
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
    pinMode(EXTERNAL_WDT_PIN, OUTPUT);
    reset_wdt();

    touchDisplayController = new TouchDisplayController();

    esp_task_wdt_config_t wdt_config = {
            .timeout_ms = 25000,
            .idle_core_mask = 0,
            .trigger_panic = true,
    };
    esp_err_t wdt_err = esp_task_wdt_init(&wdt_config);
    if (wdt_err == ESP_ERR_INVALID_STATE) {
        wdt_err = esp_task_wdt_reconfigure(&wdt_config);
    }
    esp_task_wdt_add(NULL); //add current thread to WDT watch

    twoDeviceInfo("Starting...");
    settingsManager = new SettingsManager();

    twoDeviceInfo("starting DS18D20...");
    delay(500);
    sensorController = new SensorController(ONE_WIRE_PIN, ONE_WIRE_PIN_2, settingsManager);
    settingsManager->getNavigator()->setSensorList(sensorController->buildSensorsList());

    twoDeviceInfo("Starting I2C");
    Wire.begin(I2C_SDA_PIN, I2C_SCL_PIN);
    Wire.setClock(400000);

    twoDeviceInfo("Starting heater controller...");
    heaterController = new HeaterController(settingsManager->getSettings(), sensorController,
                                            settingsManager->getNavigator());

    touchDisplayController->setHeaterController(heaterController);
    touchDisplayController->setSettingsManager(settingsManager);
    touchDisplayController->setSensorController(sensorController);

    LOGGER.info("Mounting internal flash...");
    if (!LittleFS.begin(false)) {
        LOGGER.error(" Mount Failed");
    } else
        LOGGER.info("   mounted");

    reset_wdt();
    twoDeviceInfo("starting WiFi...");
    wiFiController = new WiFiController(settingsManager);
    wiFiController->getWebServerController()->setTouchDisplayController(touchDisplayController);
    touchDisplayController->setWiFiController(wiFiController);

    twoDeviceInfo("starting MQTT...");
    mqttController = new MqttController(settingsManager);
    touchDisplayController->setMqttController(mqttController);

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

    if (mqttController)
        mqttController->handle();

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

    if ((millis() - lastHeapLogMs) >= 60000) {
        lastHeapLogMs = millis();
        char heapMsg[80];
        snprintf(heapMsg, sizeof(heapMsg), "heap free=%u min=%u",
                 (unsigned) ESP.getFreeHeap(), (unsigned) ESP.getMinFreeHeap());
        LOGGER.info(heapMsg);
    }

}