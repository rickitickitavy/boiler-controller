//
// Created by dsporykhin on 19.04.20.
//

#include <ESPmDNS.h>
#include "WiFiController.h"
#include "Defines.h"
#include "Logger.h"


WiFiController::WiFiController(SettingsManager *settingsManager) {
    this->settingsManager = settingsManager;
    this->lastConnectedTime = 10000;
    init();
}


void WiFiController::setApMode(IPAddress *ipAddress){
    WiFi.softAPsetHostname(settingsManager->getSettings()->mqttDeviceName);
    WiFi.mode(WIFI_AP);
    WiFi.softAP(settingsManager->getSettings()->mqttDeviceName, "00000000", 1, 0, 8);
    delay(10);
    WiFi.softAPConfig(*ipAddress, *ipAddress, IPAddress(255, 255, 255, 0));
}


//String WiFiController::getTextErrorStatus() {
//    station_status_t status = wifi_station_get_connect_status();
//
//    switch (status) {
//        case STATION_GOT_IP:
//            return " STATION_GOT_IP";
//        case STATION_NO_AP_FOUND:
//            return " STATION_NO_AP_FOUND";
//        case STATION_CONNECT_FAIL:
//            return " STATION_CONNECT_FAIL";
//        case STATION_WRONG_PASSWORD:
//            return " STATION_WRONG_PASSWORD";
//        case STATION_IDLE:
//            return " STATION_IDLE";
//        default:
//            return " STATION_DISCONNECTED";
//    }
//}

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
            delay(20);
        }
        if (!WiFi.isConnected()) {
            // не подключились. В режим AP

            LOGGER.error("Not connected. Switching to AP mode...");

            WiFi.mode(WIFI_OFF);
            delay(300);

            WiFi.hostname(String(settings->mqttDeviceName));

            IPAddress ipAddress = IPAddress(192, 168, 0, 1);
            WiFi.mode(WIFI_AP);
            delay(300);
            WiFi.softAP(String(String(settings->mqttDeviceName) + "-WiFi").c_str(), "00000000");
            delay(20);
            WiFi.softAPConfig(ipAddress, ipAddress, IPAddress(255, 255, 255, 0));
            delay(20);
            MDNS.begin(settings->mqttDeviceName);
            MDNS.addService("http", "tcp", 80);
            delay(400);
            LOGGER.info("    switched to AP mode. '" + String(settings->mqttDeviceName) +
                        "-WiFi'. password '00000000' (local IP " + WiFi.softAPIP().toString() + ")");
        } else {
            Serial.println("Connected to router.");
            Serial.println("local IP " + WiFi.localIP().toString());
//        WiFi.hostname(settings->network.hostName);
//        wifi_station_set_hostname(settings->network.hostName);

            MDNS.begin(&settings->mqttDeviceName[0]);
            MDNS.addService("http", "tcp", 80);

        }


        serverController = new WebServerController(settingsManager);

        LOGGER.info("WiFi current state: " + String(WiFi.getMode()));

    }
}

bool WiFiController::isClientConnected() {
//    struct station_info *stat_info;
//    stat_info = wifi_softap_get_station_info();
//    return stat_info != NULL;
    return WiFi.isConnected();
}

void WiFiController::checkConnection() {
    if (millis() - lastConnectedTime > 40000) {
        if (!WiFi.isConnected() && !isClientConnected()) {
            LOGGER.warning("trying to reconnect to AP");

            ESP.restart();

            this->lastConnectedTime = millis();

        } else {
            this->lastConnectedTime = millis();
        }
    }
}