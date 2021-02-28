#include "SettingsNavigator.h"//
#include "Logger.h"
// Created by dsporykhin on 23.04.20.
//

#include <EEPROM.h>
#include <FS.h>

SettingsManager::SettingsManager(){

    EEPROM.begin(4096);
    LOGGER.info("Load settings...");
    readSettings();
    navigator = new SettingsNavigator(this);

    if ((settings.initMarker[0] != GLOBAL_SETTINGS_MARKER_0)
        || (settings.initMarker[1] != GLOBAL_SETTINGS_MARKER_1)
        || (settings.initMarker[2] != GLOBAL_SETTINGS_MARKER_2)
        || (settings.initMarker[3] != GLOBAL_SETTINGS_MARKER_3)) {
        // настройки не инициализированы
        settings.initMarker[0] = GLOBAL_SETTINGS_MARKER_0;
        settings.initMarker[1] = GLOBAL_SETTINGS_MARKER_1;
        settings.initMarker[2] = GLOBAL_SETTINGS_MARKER_2;
        settings.initMarker[3] = GLOBAL_SETTINGS_MARKER_3;

        LOGGER.error("Settings has never been initialized. Initializing by default config...");

        settings.version = GLOBAL_CURRENT_SETTINGS_VERSION;

        // Заполнение дефолтными значениями
        resetWiFi();

        settings.mqttPort = 1883;
        memset(settings.mqttServer, 0, 12);
        memcpy(settings.mqttServer, String("192.168.2.2").c_str(), 11);
        settings.mqttReconnectIntervalMs = 1000;

        memset(settings.mqttDeviceName, 0, 7);
        memcpy(settings.mqttDeviceName, String("device").c_str(), 6);

        memcpy(settings.deviceStateOutgoingTopicPrefix, String("state").c_str(), 5);
        settings.deviceStateOutgoingTopicPrefix[5] = 0;

        memcpy(settings.deviceIncomingCommandTopicPrefix, String("command").c_str(), 7);
        settings.deviceIncomingCommandTopicPrefix[7] = 0;

        memcpy(settings.deviceIHaveBornTopic, String("deviceReady").c_str(), 11);
        settings.deviceIHaveBornTopic[11] = 0;

        memcpy(settings.mqttServerBornTopic, String("homeassistant/status").c_str(), 20);
        settings.mqttServerBornTopic[20] = 0;

        logSettings();

        saveSetting(true);
        LOGGER.warning("Settings has never been initialized");
    } else {

        LOGGER.info("Settings loaded. Version " + String(settings.version));
        if (settings.version != GLOBAL_CURRENT_SETTINGS_VERSION) {

            LOGGER.warning("Upgrade settings to version " + String(GLOBAL_CURRENT_SETTINGS_VERSION));


            settings.version = GLOBAL_CURRENT_SETTINGS_VERSION;
            LOGGER.warning(" Upgrade settings finished");
            saveSetting(false);
        }
    }


    logSettings();
};
//--------------------------------------------------------------------

SettingsNavigator* SettingsManager::getNavigator(){
    return navigator;
}
//--------------------------------------------------------------------

void SettingsManager::readSettings(GlobalSettings* settings) {
    char *bufPtr = (char *) settings;
    LOGGER.info("Loading " + String((int)sizeof(GlobalSettings)) + " bytes");
    for (int i = 0; i < sizeof(GlobalSettings); i++) {
        bufPtr[i] = EEPROM.read(i);
        if ((i % 100) == 0) {
            wdt_reset();
        }
    }
    LOGGER.info("Settings read");
}
//--------------------------------------------------------------------

void SettingsManager::readSettings() {
    readSettings(&settings);
}
//--------------------------------------------------------------------

void SettingsManager::saveSetting(bool restart) {
    saveSetting(&settings, restart);
}
//--------------------------------------------------------------------

void SettingsManager::saveSetting(GlobalSettings* settingsToSave, bool restart) {
    LOGGER.info(" Saving settings ...");

    char *dataPtr = (char *) settingsToSave;
    for (int addr = 0; addr < sizeof(GlobalSettings); addr++) {
        EEPROM.write(addr, dataPtr[addr]);
        wdt_reset();
    }
    EEPROM.commit();
    wdt_reset();
    LOGGER.info("saved");

    delay(20);
    if (restart) {
        LOGGER.warning("RESTARTING...");
        LOGGER.saveLogFile();
        SPIFFS.end();
        ESP.restart();
    }
}

//--------------------------------------------------------------------

void SettingsManager::resetWiFi() {
    String temp = WIFI_DEFAULT_SID;
    temp.toCharArray(&settings.network.ssid[0], sizeof(settings.network.ssid));

    temp = WIFI_DEFAULT_PASSWORD;
    temp.toCharArray(&settings.network.password[0], sizeof(settings.network.password));

    temp = WIFI_DEFAULT_HOST_NAME;
    temp.toCharArray(&settings.network.hostName[0], sizeof(settings.network.hostName));
}
//--------------------------------------------------------------------

GlobalSettings* SettingsManager::getSettings(){
    return &settings;
}
//--------------------------------------------------------------------

void SettingsManager::logSettings() {
    LOGGER.info("----- SETTINGS ----");
    LOGGER.info("   wifw:");
    LOGGER.info("      SSID: " + String(settings.network.ssid));
    LOGGER.info("      password: " + String(settings.network.password));
    LOGGER.info("      host: " + String(settings.network.hostName));
    LOGGER.info("   mqtt:");
    LOGGER.info("      server: " + String(settings.mqttServer));
    LOGGER.info("      port: " + String(settings.mqttPort));
    LOGGER.info("      reconIntervalMs: " + String(settings.mqttReconnectIntervalMs));
    LOGGER.info("      device name: " + String(settings.mqttDeviceName));
    LOGGER.info("   device: " + String(settings.mqttDeviceName));
    LOGGER.info("      have born topic: " + String(settings.deviceIHaveBornTopic));
    LOGGER.info("      outgoing topic prefix: " + String(settings.deviceStateOutgoingTopicPrefix));
    LOGGER.info("      in command topic prefix: " + String(settings.deviceIncomingCommandTopicPrefix));
    LOGGER.info("      server has born topic: " + String(settings.mqttServerBornTopic));
}
//--------------------------------------------------------------------
