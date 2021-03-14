//
// Created by dsporykhin on 19.04.20.
//

#include <ESP8266WiFi.h>
#include <ESP8266mDNS.h>
#include "WiFiController.h"
#include "Defines.h"
#include "Logger.h"


WiFiController::WiFiController(SettingsManager *settingsManager) {
    this->settingsManager = settingsManager;
    init();
}

String WiFiController::getTextErrorStatus() {
    station_status_t status = wifi_station_get_connect_status();

    switch(status) {
        case STATION_GOT_IP:
            return "   STATION_GOT_IP";
        case STATION_NO_AP_FOUND:
            return"   STATION_NO_AP_FOUND";
        case STATION_CONNECT_FAIL:
            return"   STATION_CONNECT_FAIL";
        case STATION_WRONG_PASSWORD:
            return"   STATION_WRONG_PASSWORD";
        case STATION_IDLE:
            return"   STATION_IDLE";
        default:
            return"   STATION_DISCONNECTED";
    }
}

void WiFiController::init() {
    GlobalSettings *settings = settingsManager->getSettings();

    LOGGER.info("WiFi is persistent: " + String(WiFi.getPersistent()) + "\r\n");

    if ((WiFi.hostname() != String(settings->network.hostName))
        || (WiFi.softAPSSID() != settings->network.ssid)
        || (WiFi.softAPPSK() != String(settings->network.password))
            || (WiFi.getMode() != WIFI_STA)) {

        LOGGER.info("Configuring WiFi...");

        wdt_reset();

        WiFi.mode(WIFI_OFF);
        delay(300);

        WiFi.hostname(String(settings->mqttDeviceName));
        wifi_station_set_hostname(settings->mqttDeviceName);

//        WiFi.mode(settings->network.wifiMode == 1 ? WIFI_AP : WIFI_STA);
        WiFi.mode(WIFI_STA);
        WiFi.begin(settings->network.ssid, settings->network.password);

        long startedAt = millis();
        while (!WiFi.isConnected() && (millis() - startedAt < 10000)) {
            delay(20);
        }
        if (!WiFi.isConnected()) {
            // не подключились. В режим AP

            LOGGER.error("Not connected. " + getTextErrorStatus() + ". Switching to AP mode...");

            wdt_reset();

            WiFi.mode(WIFI_OFF);
            delay(300);

            WiFi.hostname(String(settings->mqttDeviceName));
            wifi_station_set_hostname(settings->mqttDeviceName);

            IPAddress ipAddress = IPAddress(192, 168, 0, 1);
            WiFi.mode(WIFI_AP);
            delay(300);
            WiFi.softAP(String(settings->mqttDeviceName) + "-WiFi", "00000000");
            delay(20);
            WiFi.softAPConfig(ipAddress, ipAddress, IPAddress(255, 255, 255, 0));
            delay(20);
            MDNS.begin(String(settings->mqttDeviceName));
            MDNS.addService("http", "tcp", 80);
            delay(400);
            LOGGER.info("    switched to AP mode. '" + String(settings->mqttDeviceName) + "-WiFi'. password '00000000' (local IP " + WiFi.softAPIP().toString() + ")");
        } else {
            Serial.println("Connected to router.");
            Serial.println("local IP " + WiFi.localIP().toString());
//        WiFi.hostname(settings->network.hostName);
//        wifi_station_set_hostname(settings->network.hostName);

        MDNS.begin(String(&settings->network.hostName[0]));
        MDNS.addService("http", "tcp", 80);

        }


        serverController = new WebServerController(settingsManager);

        LOGGER.info("WiFi current state: " + String(WiFi.getMode()));

    }
}