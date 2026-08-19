//
// Created by dsporykhin on 19.04.20.
//

#include <ESPmDNS.h>
#include <esp_task_wdt.h>
#include "WiFiController.h"

WiFiController::WiFiController(SettingsManager *settingsManager) {
    this->settingsManager = settingsManager;
    this->lastConnectedTime = 10000;
    init();
}

void WiFiController::initNTP() {
    char date_str[128];

    time_t epochTime = time(0);
    tm *localtm = localtime(&epochTime);

    sprintf(date_str, "Date time before %s", asctime(localtm));
    LOGGER.info(date_str);

    ntpUDP = new WiFiUDP();
    timeClient = new NTPClient(*ntpUDP);

    timeClient->begin();
    timeClient->setTimeOffset(3600 * 3);
    timeClient->setUpdateInterval(1800000);

    long started_at = millis();
    while (!timeClient->update() && ((millis() - started_at) < 5000)) {
        timeClient->forceUpdate();
    }

    if (timeClient->update()) {
        LOGGER.info("Data is " + timeClient->getFormattedDate());
        LOGGER.info("Time is " + timeClient->getFormattedTime());

        epochTime = timeClient->getEpochTime();
        localtm = localtime(&epochTime);

        sprintf(date_str, "Set time as %s", asctime(localtm));
        LOGGER.info(date_str);

        struct timeval now = {.tv_sec = epochTime};
        settimeofday(&now, NULL);

        if (getLocalTime(localtm, 0)) {
            sprintf(date_str, "current date and time is %s (millis is %i)", asctime(localtm), millis());
            LOGGER.info(date_str);
        }

    } else
        LOGGER.error("Failed to get current date and time");
}

void WiFiController::setApMode(IPAddress *ipAddress) {
    WiFi.softAPsetHostname(settingsManager->getSettings()->mqttDeviceName);
    WiFi.mode(WIFI_AP);
    WiFi.softAP(settingsManager->getSettings()->mqttDeviceName, "00000000", 1, 0, 8);
    delay(10);
    WiFi.softAPConfig(*ipAddress, *ipAddress, IPAddress(255, 255, 255, 0));
}

void WiFiController::init() {
    GlobalSettings *settings = settingsManager->getSettings();

    if ((!WiFi.hostname(settings->mqttDeviceName))
        || (WiFi.softAPSSID() != settings->network.ssid)
        //        || (WiFi.softAPPSK() != String(settings->network.password))
        || (WiFi.getMode() != WIFI_STA)) {

        LOGGER.info("Configuring WiFi...");

        WiFi.hostname(String(settings->mqttDeviceName));

//        WiFi.mode(settings->network.wifiMode == 1 ? WIFI_AP : WIFI_STA);
        WiFi.mode(WIFI_STA);
        WiFi.begin(settings->network.ssid, settings->network.password);

        long startedAt = millis();
        while (!WiFi.isConnected() && (millis() - startedAt < 10000)) {
            esp_task_wdt_reset();
            delay(20);
        }
        if (!WiFi.isConnected()) {
            // не подключились. В режим AP

            LOGGER.error("Not connected. Switching to AP mode...");

            WiFi.mode(WIFI_OFF);
            esp_task_wdt_reset();
            delay(300);

            WiFi.hostname(String(settings->mqttDeviceName));

            IPAddress ipAddress = IPAddress(192, 168, 0, 1);
            WiFi.mode(WIFI_AP);
            esp_task_wdt_reset();
            delay(300);
            WiFi.softAP(String(String(settings->mqttDeviceName) + "-WiFi").c_str(), "00000000");
            esp_task_wdt_reset();
            delay(20);
            WiFi.softAPConfig(ipAddress, ipAddress, IPAddress(255, 255, 255, 0));
            delay(20);
            MDNS.begin(settings->mqttDeviceName);
            MDNS.addService("http", "tcp", 80);
            esp_task_wdt_reset();
            delay(400);
            LOGGER.info("    switched to AP mode. '" + String(settings->mqttDeviceName) +
                        "-WiFi'. password '00000000' (local IP " + WiFi.softAPIP().toString() + ")");
        } else {
            Serial.println("Connected to router.");
            Serial.println("local IP " + WiFi.localIP().toString());

            MDNS.begin(&settings->mqttDeviceName[0]);
            MDNS.addService("http", "tcp", 80);

            LOGGER.info("Getting real date and time from NTP");
//            initNTP();

        }


        serverController = new WebServerController(settingsManager);

        LOGGER.info("WiFi current state: " + String(WiFi.getMode()));

    }
}

void WiFiController::setHeaterController(HeaterController *heaterController) {
    this->serverController->heaterController = heaterController;
}

WebServerController *WiFiController::getWebServerController() {
    return serverController;
}

bool WiFiController::isClientConnected() {
    return WiFi.isConnected();
}

void WiFiController::reapplyNetworkSettings() {
    GlobalSettings *globalSettings = settingsManager->getSettings();
    LOGGER.info("Reapplying WiFi settings (no restart)...");

    // Already on the configured router — avoid tear-down that races AsyncWebServer/MQTT and
    // can panic inside wifi_softap_start when STA reconnect fails.
    if (WiFi.getMode() == WIFI_STA && WiFi.isConnected()
        && WiFi.SSID() == String(globalSettings->network.ssid)) {
        WiFi.setHostname(globalSettings->mqttDeviceName);
        LOGGER.info("WiFi already connected to configured SSID; skip reconnect.");
        return;
    }

    WiFi.disconnect(false, false);
    delay(100);
    esp_task_wdt_reset();
    WiFi.mode(WIFI_STA);
    WiFi.setHostname(globalSettings->mqttDeviceName);
    WiFi.begin(globalSettings->network.ssid, globalSettings->network.password);

    long startedAt = millis();
    while (!WiFi.isConnected() && (millis() - startedAt < 10000)) {
        esp_task_wdt_reset();
        delay(20);
    }
    if (!WiFi.isConnected()) {
        LOGGER.error("Reconnect failed; switching to AP mode...");
        // Match init() sequencing — abrupt softAP after WIFI_OFF caused LoadProhibited.
        WiFi.mode(WIFI_OFF);
        esp_task_wdt_reset();
        delay(300);
        IPAddress ipAddress = IPAddress(192, 168, 0, 1);
        WiFi.mode(WIFI_AP);
        esp_task_wdt_reset();
        delay(300);
        WiFi.softAP(String(String(globalSettings->mqttDeviceName) + "-WiFi").c_str(), "00000000");
        esp_task_wdt_reset();
        delay(20);
        WiFi.softAPConfig(ipAddress, ipAddress, IPAddress(255, 255, 255, 0));
        delay(20);
    } else {
        LOGGER.info("Reconnected. IP " + WiFi.localIP().toString());
    }
}

void WiFiController::checkConnection() {
//    if (millis() - lastConnectedTime > 40000) {
//        if (!WiFi.isConnected() && !isClientConnected()) {
//            LOGGER.warning("trying to reconnect to AP");
//
//            ESP.restart();
//
//            this->lastConnectedTime = millis();
//
//        } else {
//            this->lastConnectedTime = millis();
//        }
//    }
}