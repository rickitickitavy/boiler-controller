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

    LOGGER.info("------------------ read ------------- ");
    LOGGER.info("      scan_interval_ms: " + String(settings.heaterSettings.scan_interval_ms));
    LOGGER.info("      SMA_temperature_period_sec: " + String(settings.heaterSettings.temperatureSettings.SMA_temperature_period_sec));

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
        memcpy(settings.mqttServer, String("192.168.4.254").c_str(), 13);
        settings.mqttReconnectIntervalMs = 1000;

        memset(settings.mqttDeviceName, 0, 8);
        memcpy(settings.mqttDeviceName, String("heater-test").c_str(), 7);

        memcpy(settings.deviceStateOutgoingTopicPrefix, String("state").c_str(), 5);
        settings.deviceStateOutgoingTopicPrefix[5] = 0;

        memcpy(settings.deviceIncomingCommandTopicPrefix, String("command").c_str(), 8);
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
        settings.heaterSettings.servos_hardware_settings.oxygen_servo_settings.working_max_angle = 180;

        settings.heaterSettings.capacities_setting.boiler_ltr = 200;
        settings.heaterSettings.capacities_setting.heater_core_ltr = 100;
        settings.heaterSettings.capacities_setting.accumulator_ltr = 1000;
        settings.heaterSettings.capacities_setting.pipes_and_radiators_ltr = 100;

        settings.heaterSettings.scan_interval_ms = 3000;

        settings.heaterSettings.two_pumps_settings.enabled = true;
        settings.heaterSettings.two_pumps_settings.start_on_delta_temperature_between_input_and_output = 18;
        settings.heaterSettings.two_pumps_settings.stop_on_delta_temperature_between_input_and_output = 10;
        settings.heaterSettings.two_pumps_settings.start_on_delta_temperature_between_core_and_input = 28;
        settings.heaterSettings.two_pumps_settings.stop_on_delta_temperature_between_core_and_input = 20;
        settings.heaterSettings.two_pumps_settings.flow_senser_installed = false;
        settings.heaterSettings.two_pumps_settings.first_pump_flow_litters_per_minute = 14;
        settings.heaterSettings.two_pumps_settings.second_pump_flow_litters_per_minute = 14;
        settings.heaterSettings.two_pumps_settings.flow_sensor_ticks_per_litters = 27;


        settings.heaterSettings.stadbyCoolingSettings.start_warming_cycle_on_power = 1000;
        settings.heaterSettings.coolingByPumpsSettings.min_delta_btw_core_and_input_to_start_pumps = 10;
        settings.heaterSettings.coolingByPumpsSettings.start_pumps_temperature = 90;
        settings.heaterSettings.coolingByPumpsSettings.delta_btw_start_and_core_to_stop_pumps = 5;

        settings.heaterSettings.temperatureSettings.core_power_when_need_to_refuel = 20000;
        settings.heaterSettings.temperatureSettings.core_target = 92;
        settings.heaterSettings.temperatureSettings.core_critical = 99;
        settings.heaterSettings.temperatureSettings.thermal_accumulator_target = 85;
        settings.heaterSettings.temperatureSettings.core_overheat = 96;

        settings.heaterSettings.temperatureSettings.core_power_EMA = 6;
        settings.heaterSettings.temperatureSettings.core_power_diff_EMA = 6;
        settings.heaterSettings.temperatureSettings.core_temp_diff_EMA = 6;
        settings.heaterSettings.temperatureSettings.SMA_temperature_period_sec = 60;

        settings.heaterSettings.warming_settings.time_to_reach_target_power_sec = 320;
        settings.heaterSettings.warming_settings.start_pid_temperature = 70;
        settings.heaterSettings.warming_settings.target_power_to_switch_to_the_PID_mode = 14900;
        settings.heaterSettings.warming_settings.smoke_door_value_prcnt = 66;
        settings.heaterSettings.warming_settings.upper_door_value_prcnt = 50;

        settings.heaterSettings.finalCoolingSettings.max_temperature_to_switch_to_standBy = 70;
        settings.heaterSettings.finalCoolingSettings.delay_to_switch_to_standby_mode_sec = 900;
        settings.heaterSettings.finalCoolingSettings.power_to_switch_to_warming_mode = 6500;

        settings.heaterSettings.burning_settings.p = 5.2;
        settings.heaterSettings.burning_settings.i = 0.3;
        settings.heaterSettings.burning_settings.d = 110;
        settings.heaterSettings.burning_settings.max_i = 15;
        settings.heaterSettings.burning_settings.min_i = -15;
        settings.heaterSettings.burning_settings.upper_door_value_prcnt = 100;
        settings.heaterSettings.burning_settings.smoke_door_value_prcnt = 60;
        settings.heaterSettings.burning_settings.power_to_switch_to_warming_mode = 15000;
        settings.heaterSettings.burning_settings.oxygen_door_val_to_warming_mode = 25;

        settings.heaterSettings.modellerSettings.max_core_power = 40000;
        settings.heaterSettings.modellerSettings.core_energy_transmitting_coef = 0.5;
        settings.heaterSettings.modellerSettings.skip_first_N_cycles = 50;
        settings.heaterSettings.modellerSettings.ema_doors_reactions = 200;
        settings.heaterSettings.modellerSettings.radiators_base_temperature = 70;
        settings.heaterSettings.modellerSettings.radiators_efficiensy_coef_pow = 1.25;
        settings.heaterSettings.modellerSettings.radiators_home_temperature = 25;
        settings.heaterSettings.modellerSettings.additional_core_doors_coef = 1.7;
        settings.heaterSettings.modellerSettings.radiators_flow_lpm = 10;
        settings.heaterSettings.modellerSettings.logging_modeller_info = false;
        settings.heaterSettings.modellerSettings.power_fade_out_steps = 1000;
        settings.heaterSettings.modellerSettings.naturaly_cooling_coefficient_as_devider = 10000;
        settings.heaterSettings.modellerSettings.naturaly_cooling_pow = 1.5;


        settings.heaterSettings.modellerSettings.stages[0].active = true;
        settings.heaterSettings.modellerSettings.stages[0].initial_angle = 0;
        settings.heaterSettings.modellerSettings.stages[0].core_power_grow_angle_step = 0.1;
        settings.heaterSettings.modellerSettings.stages[0].max_power_cycles = 7200;
        settings.heaterSettings.modellerSettings.stages[0].radiators_base_power = 10000;
        settings.heaterSettings.modellerSettings.stages[0].radiator_stop_after_cycle = 5000;
        settings.heaterSettings.modellerSettings.stages[0].length_of_modeling_cycles = 17000;
        settings.heaterSettings.modellerSettings.stages[0].tempr_start_acc_top = 0;
        settings.heaterSettings.modellerSettings.stages[0].tempr_start_acc_higher = 0;
        settings.heaterSettings.modellerSettings.stages[0].tempr_start_acc_lower = 0;
        settings.heaterSettings.modellerSettings.stages[0].tempr_start_acc_bottom = 0;
        settings.heaterSettings.modellerSettings.stages[0].tempr_start_core = 0;
        settings.heaterSettings.modellerSettings.stages[0].tempr_start_core_volume = 0;

        for (int stage_index = 1; stage_index < 5; stage_index++) {
            settings.heaterSettings.modellerSettings.stages[stage_index].active = false;
            settings.heaterSettings.modellerSettings.stages[stage_index].initial_angle = 0;
            settings.heaterSettings.modellerSettings.stages[stage_index].core_power_grow_angle_step = 0.1;
            settings.heaterSettings.modellerSettings.stages[stage_index].max_power_cycles = 10000;
            settings.heaterSettings.modellerSettings.stages[stage_index].radiators_base_power = 20000;
            settings.heaterSettings.modellerSettings.stages[stage_index].radiator_stop_after_cycle = 5000;
            settings.heaterSettings.modellerSettings.stages[stage_index].length_of_modeling_cycles = 17000;
            settings.heaterSettings.modellerSettings.stages[stage_index].tempr_start_acc_top = 0;
            settings.heaterSettings.modellerSettings.stages[stage_index].tempr_start_acc_higher = 0;
            settings.heaterSettings.modellerSettings.stages[stage_index].tempr_start_acc_lower = 0;
            settings.heaterSettings.modellerSettings.stages[stage_index].tempr_start_acc_bottom = 0;
            settings.heaterSettings.modellerSettings.stages[stage_index].tempr_start_core = 0;
            settings.heaterSettings.modellerSettings.stages[stage_index].tempr_start_core_volume = 0;
        }

        settings.version = 2;
        settings.send_data_to_mqtt_interval_ms = 6000;

        memcpy(settings.telemetrySettings.catName, "telemetry\0", 10);
        settings.telemetrySettings.flush_interval_ms = 240000;
        settings.telemetrySettings.flush_inteval_records = 40;
        settings.telemetrySettings.max_file_size_bytes = 20 * 1024 * 1024;
        settings.telemetrySettings.log_gebug_to_UART = false;

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
    servoHardwareSettings->min_impulse_length_us = 600;
    servoHardwareSettings->max_impulse_length_us = 2600;
    servoHardwareSettings->working_min_angle = 0;
    servoHardwareSettings->working_max_angle = 90;
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
    LOGGER.info("      scan_interval_ms: " + String(settings.heaterSettings.scan_interval_ms));
    LOGGER.info("      SMA_temperature_period_sec: " + String(settings.heaterSettings.temperatureSettings.SMA_temperature_period_sec));
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
    LOGGER.info("   sensors scan interval (ms): " + String(settings.heaterSettings.scan_interval_ms));

}
//--------------------------------------------------------------------


//--------------------------------------------------------------------
