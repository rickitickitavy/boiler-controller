#include "SettingsNavigator.h"//
#include "Logger.h"
#include "Converter.h"
#include "../../../../../.platformio/packages/framework-arduinoespressif8266/cores/esp8266/Esp.h"
// Created by dsporykhin on 23.04.20.
//

#include <EEPROM.h>
#include <FS.h>
#include <SPIFFS.h>

SettingsManager::SettingsManager(){

    EEPROM.begin(4096);
    LOGGER.info("Load settings...");
    readSettings();
    LOGGER.info("b0 = " + String((byte)settings.initMarker[0]));
    LOGGER.info("b1 = " + String((byte)settings.initMarker[1]));
    LOGGER.info("b2 = " + String((byte)settings.initMarker[2]));
    LOGGER.info("b3 = " + String((byte)settings.initMarker[3]));
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
        memset(settings.mqttServer, 0, 13);
        memcpy(settings.mqttServer, String("192.168.4.20").c_str(), 12);
        settings.mqttReconnectIntervalMs = 1000;

        memset(settings.mqttDeviceName, 0, 8);
        memcpy(settings.mqttDeviceName, String("heater2").c_str(), 7);

        memcpy(settings.deviceStateOutgoingTopicPrefix, String("state").c_str(), 5);
        settings.deviceStateOutgoingTopicPrefix[5] = 0;

        memcpy(settings.deviceIncomingCommandTopicPrefix, String("commands").c_str(), 8);
        settings.deviceIncomingCommandTopicPrefix[7] = 0;

        memcpy(settings.deviceIHaveBornTopic, String("deviceReady").c_str(), 11);
        settings.deviceIHaveBornTopic[11] = 0;

        memcpy(settings.mqttServerBornTopic, String("homeassistant/status").c_str(), 20);
        settings.mqttServerBornTopic[20] = 0;

        memset(&settings.mqttInputToolTopic, 0, 32);
        memset(&settings.mqttOutputToolTopic, 0, 32);

        settings.defaultSwitcherState = false;
        settings.dimmerPwmValue = 50;

        memset(settings.ds18D20Addresses, 0, sizeof(settings.ds18D20Addresses));

        initServoConfig(&settings.heaterSettings.servos_hardware_settings.oxygen_servo_settings);
        initServoConfig(&settings.heaterSettings.servos_hardware_settings.smoke_servo_settings);
        initServoConfig(&settings.heaterSettings.servos_hardware_settings.upper_door_servo_settings);

        settings.heaterSettings.capacities_setting.boiler_ltr = 200;
        settings.heaterSettings.capacities_setting.heater_core_ltr = 50;
        settings.heaterSettings.capacities_setting.accumulator_ltr = 1000;
        settings.heaterSettings.capacities_setting.pipes_and_radiators_ltr = 100;

        settings.heaterSettings.scan_interval_ms = 3000;

        settings.heaterSettings.two_pumps_settings.enabled = true;
        settings.heaterSettings.two_pumps_settings.start_on_delta_temperature_between_input_and_output = 18;
        settings.heaterSettings.two_pumps_settings.stop_on_delta_temperature_between_input_and_output = 10;
        settings.heaterSettings.two_pumps_settings.start_on_delta_temperature_between_core_and_input = 28;
        settings.heaterSettings.two_pumps_settings.stop_on_delta_temperature_between_core_and_input = 20;

        settings.heaterSettings.temperatureSettings.start_burn_cycle_on_temperature_up_to = 70;
        settings.heaterSettings.temperatureSettings.core_low = 70;
        settings.heaterSettings.temperatureSettings.core_target = 92;
        settings.heaterSettings.temperatureSettings.core_critical = 98;
        settings.heaterSettings.temperatureSettings.start_burn_cycle_on_temperature_up_to = 70;

        settings.heaterSettings.oxygen_pid.p = 1;
        settings.heaterSettings.oxygen_pid.i = 0.01;
        settings.heaterSettings.oxygen_pid.d = 100;
        settings.heaterSettings.oxygen_pid.max_i = 15;
        settings.heaterSettings.oxygen_pid.start_pid_on_temperature_up_to = 60;
        settings.heaterSettings.oxygen_pid.min_output_value_prcnt = 0;
        settings.heaterSettings.oxygen_pid.max_output_value_prcnt = 100;


        settings.version = 2;
        settings.send_data_to_mqtt_interval_ms = 3000;

        logSettings();

        saveSetting(true);
        LOGGER.warning("Settings has never been initialized");
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

void SettingsManager::initServoConfig(ServoHardwareSettings *servoHardwareSettings){
    servoHardwareSettings->total_degrees = 180;
    servoHardwareSettings->min_impulse_length_us = 800;
    servoHardwareSettings->max_impulse_length_us = 2000;
    servoHardwareSettings->working_min_angle = 0;
    servoHardwareSettings->working_max_degrees = 35;
}
//--------------------------------------------------------------------

void SettingsManager::saveSetting(GlobalSettings* settingsToSave, bool restart) {
    LOGGER.info(" Saving settings ...");

    char *dataPtr = (char *) settingsToSave;
    for (int addr = 0; addr < sizeof(GlobalSettings); addr++) {
        EEPROM.write(addr, dataPtr[addr]);
    }
    EEPROM.commit();
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
    temp.toCharArray(&settings.mqttDeviceName[0], sizeof(settings.mqttDeviceName));
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
    LOGGER.info("      host: " + String(settings.mqttDeviceName));
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

    LOGGER.info("   sensors: ");
    LOGGER.info("      heater: ");

    char buffer[SENSORS_ADDR_SIZE * 2 + 1];
    Converter::bytesToAsciiHex(buffer, (uint8_t*)&settings.ds18D20Addresses[SENSORS_ADDR_SIZE * 0], SENSORS_ADDR_SIZE);
    LOGGER.info("         core: " + String(buffer));
    Converter::bytesToAsciiHex(buffer, (uint8_t*)&settings.ds18D20Addresses[SENSORS_ADDR_SIZE * 1], SENSORS_ADDR_SIZE);
    LOGGER.info("         output flow: " + String(buffer));
    Converter::bytesToAsciiHex(buffer, (uint8_t*)&settings.ds18D20Addresses[SENSORS_ADDR_SIZE * 2], SENSORS_ADDR_SIZE);
    LOGGER.info("         input flow: " + String(buffer));
    LOGGER.info("      termoaccumulator: ");
    Converter::bytesToAsciiHex(buffer, (uint8_t*)&settings.ds18D20Addresses[SENSORS_ADDR_SIZE * 3], SENSORS_ADDR_SIZE);
    LOGGER.info("         top: " + String(buffer));
    Converter::bytesToAsciiHex(buffer, (uint8_t*)&settings.ds18D20Addresses[SENSORS_ADDR_SIZE * 4], SENSORS_ADDR_SIZE);
    LOGGER.info("         middle: " + String(buffer));
    Converter::bytesToAsciiHex(buffer, (uint8_t*)&settings.ds18D20Addresses[SENSORS_ADDR_SIZE * 5], SENSORS_ADDR_SIZE);
    LOGGER.info("         bottom: " + String(buffer));

//    LOGGER.info("   controllers:");
//    LOGGER.info("      PWM I2C controller addr: " + String(settings.pwm_controller_address));
//    LOGGER.info("   servos:");
//    LOGGER.info("      smoke door: " + String(settings.smoke_pipe_control_channel_id));
//    LOGGER.info("      oxygen door: " + String(settings.oxygen_door_control_channel_id));
//    LOGGER.info("      upper door: " + String(settings.upper_door_control_channel_id));
}
//--------------------------------------------------------------------


//--------------------------------------------------------------------
