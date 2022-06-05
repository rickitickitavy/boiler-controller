//
// Created by dsporykhin on 25.03.22.
//

#ifndef BASE_ESP8266_MQTT_HEATERCONTROLLER_H
#define BASE_ESP8266_MQTT_HEATERCONTROLLER_H


#include <cstdint>
#include "lib/servo/Servo.h"
#include "GlobalSettings.h"
#include "SensorController.h"
#include "PID.h"
#include "Telemetry.h"
#include "PumpController.h"
#include "PumpsController.h"
#include "ServoController.h"


#define MODE_WARMING_TIME_TO_WAIT_FOR_REACHED_PID_MODE_SEC 1200

/**
 * Map of states
 *
 * STAND_BY -> WARMING
 *
 * WARMING -> PID_ACTIVE
 *         -> FINAL_COOLING
 *
 * PID_ACTIVE -> FINAL_COOLING
 *            -> OVERHEATED
 *
 * FINAL_COOLING -> STAND_BY
 *               -> WARMING
 *
 * OVERHEATED -> CRITICAL
 *            -> PID_ACTIVE
 *
 * CRITICAL -> OVERHEATED
 *          -> ALARM (BANG)
 */
enum HeaterMode {
    DOOR_OPENED = 128,
    STAND_BY = 1,
    WARMING = 2,
    PID = 4,
    FINAL_COOLING = 8,
    OVERHEATED = 16,
    CRITICAL = 32
};

class HeaterController {
private:
    GlobalSettings *settings;
    HeaterSettings *heaterSettings;
    SensorController *sensorController;
    SettingsNavigator *settingsNavigator;
    Telemetry *telemetry;

    ServoController *smoke_pipe_control;
    ServoController *oxygen_door_control;
    ServoController *upper_door_control;

    PumpsController *pumpsController;

    PID *oxygen_pid;
    long last_cycle_time;
    long last_cycle_length;
    double previous_core_temperature;
    double core_DEMA_temperature;

    double previous_core_power;
    double current_core_power;
    double core_EMA_power;
    double core_DEMA_power;

    double core_flow;
    long flow_ticks;


    HeaterMode mode;

    long DiffEMA_down_bellow_zero_at;
    long DiffEMA_rose_above_zero_at;

    /**
     * calc core flow, core_DEMA_temperature, current_core_power, core_DEMA_power, core_EMA_power,
     * DiffEMA_down_bellow_zero_at, DiffEMA_rose_above_zero_at
     *
     */
    void calcMainCoreCharacteristics();

    /**
     * returns time in seconds of how long DEMA is less than zero
     * @return
     */
    long getTimeDEMABellowZeroSec();

    /**
     * returns time in seconds of how long DEMA is grater or equals zero
     * @return
     */
    long getTimeDEMAAboveZeroSec();

    /**
     * reset al timers to zero
     */
    void resetDEMAtimers();

    bool two_pump_active_delta_core_output;
    bool two_pump_active_delta_core_input;
    void handleTwoPumps();
    void handlePumps();

    void handleModes();

    void handle_STAND_BY_mode();
    void enterTo_STAND_BY_mode();

    void enterTo_WARMING_mode();
    void handle_WARMING_mode();

    void handle_PID_mode();
    void enterTo_PID_mode();

    void handle_FINAL_COOLING_mode();
    void enterTo_FINAL_COOLING_mode();

    void handle_DOOR_OPENED_mode();

    void handle_OVERHEATED_mode();
    void enterTo_OVERHEATED_mode();

    void handle_CRITICAL_mode();
    void enterTo_CRITICAL_mode();

    void collectTelemetry();


public:
    HeaterController(GlobalSettings *settings,
                     SensorController *sensorController, SettingsNavigator *settingsNavigator);

    void handle();
};


#endif //BASE_ESP8266_MQTT_HEATERCONTROLLER_H
