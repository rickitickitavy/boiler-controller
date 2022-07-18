//
// Created by dsporykhin on 25.03.22.
//

#ifndef BASE_ESP8266_MQTT_HEATERCONTROLLER_H
#define BASE_ESP8266_MQTT_HEATERCONTROLLER_H


#include <cstdint>
#include "lib/servo/Servo.h"
#include "GlobalSettings.h"
#include "SensorController.h"
#include "PidRegulator.h"
#include "Telemetry.h"
#include "PumpController.h"
#include "PumpsController.h"
#include "ServoController.h"
#include "MainCoreParams.h"
#include "DoorsController.h"
#include "FlowSensor.h"


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
    FlowSensor *flowSensor;

    DoorsController *doorsController;

    PumpsController *pumpsController;

    CoreModel *coreModel;
    PidRegulator *pidRegulator;
    long last_cycle_time;
    long last_cycle_length;
    double previous_core_temperature;

    int estimated_modelling_cycle_counter;

    MainCoreParams mainCoreParams;

    double previous_core_power;

    long flow_ticks;

    bool standby_cooling_active;
    long cycle_index;

    HeaterMode mode;

    long time_to_close_oxygen_door_in_stanby_mode;

    bool modelling_is_active;


    /**
     * calc core flow, core_DEMA_temperature, current_core_power, core_DEMA_power, core_EMA_power,
     * DiffEMA_down_bellow_zero_at, DiffEMA_rose_above_zero_at
     *
     */
    void calcMainCoreCharacteristics(long last_cycle_length);

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
    void handlePumps();

    void handleModes();

    void handle_STAND_BY_mode();
    void switchTo_STAND_BY_mode();

    long entered_to_warming_mode_at;
    long entered_to_warming_mode_at_cycle_index;
    void switchTo_WARMING_mode();
    void handle_WARMING_mode();

    void handle_PID_mode();
    void switchTo_PID_mode();

    long final_cooling_power_low_at;
    void handle_FINAL_COOLING_mode();
    void switchTo_FINAL_COOLING_mode();

    void handle_DOOR_OPENED_mode();

    void handle_OVERHEATED_mode();
    void switchTo_OVERHEATED_mode();

    void handle_CRITICAL_mode();
    void switchTo_CRITICAL_mode();

    TelemetryDataRecord dataRecord;
    void collectTelemetry(long last_cycle_length);

public:
    HeaterController(GlobalSettings *settings,
                     SensorController *sensorController, SettingsNavigator *settingsNavigator);

    void handle();

    void openOxygenDoorForTime(long time_sec);

    void startModelling();
    void stopModelling();
    void getTelemetry(char *buffer);
};


#endif //BASE_ESP8266_MQTT_HEATERCONTROLLER_H
