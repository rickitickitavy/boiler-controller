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

void WiFiController::init() {
    GlobalSettings *settings = settingsManager->getSettings();

    LOGGER.info("WiFi  \r\nSID: " + String(&settings->network.ssid[0]) + "\r\n"
            " password: " + String(&settings->network.password[0]) + "\r\n"
                        " hostName: " + String(settings->network.hostName) + "\r\n"
                        " Current state: " + String(WiFi.getMode()) + "\r\n"
                        " is persistent: " + String(WiFi.getPersistent()) + "\r\n");

    if ((WiFi.hostname() != String(settings->network.hostName))
        || (WiFi.softAPSSID() != settings->network.ssid)
        || (WiFi.softAPPSK() != String(settings->network.password))
            || (WiFi.getMode() != WIFI_STA)) {

        LOGGER.info("Configuring WiFi...");

        wdt_reset();

        WiFi.hostname(settings->network.hostName);
        wifi_station_set_hostname(settings->network.hostName);
//        WiFi.mode(settings->network.wifiMode == 1 ? WIFI_AP : WIFI_STA);
        WiFi.mode(WIFI_STA);
        delay(2);

        WiFi.begin(settings->network.ssid, settings->network.password);

        long startedAt = millis();
        while (!WiFi.isConnected() && (millis() - startedAt < 10000)) {
            delay(20);
        }
        if (!WiFi.isConnected()) {
            // не подключились. В режим AP
            Serial.println("Not connected. Switching to AP mode");
            IPAddress ipAddress = IPAddress(192, 168, 0, 1);
            WiFi.mode(WIFI_AP);
            WiFi.softAP(settings->network.ssid, settings->network.password);
            WiFi.softAPConfig(ipAddress, ipAddress, IPAddress(255, 255, 255, 0));
            MDNS.begin("base");
            MDNS.addService("http", "tcp", 80);

        } else {
            Serial.println("Connected to router.");
            Serial.println("local IP " + WiFi.localIP().toString());
//        WiFi.hostname(settings->network.hostName);
//        wifi_station_set_hostname(settings->network.hostName);

//        MDNS.begin(String(&settings->network.hostName[0]));
//        MDNS.addService("http", "tcp", 80);

        }


        serverController = new WebServerController(settingsManager);

        LOGGER.info("WiFi current state: " + String(WiFi.getMode()));

    }
}