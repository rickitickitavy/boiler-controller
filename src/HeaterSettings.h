//
// Created by dsporykhin on 31.03.22.
//

#ifndef BASE_ESP8266_MQTT_HEATERSETTINGS_H
#define BASE_ESP8266_MQTT_HEATERSETTINGS_H


struct PidSettings {
    float p;
    float i;

    float d;
    float d_sma;

    float max_i;
    float min_i;
    /**
     * minimum of output value of PID
     */
    float min_output_value_prcnt;

    /**
     * diapason of vales of PID output
     */
    float max_output_value_prcnt;
};

struct VolumeCapacitiesSetting {
    /**
     * thermoaccumulator volume in litters
     */
    int accumulator_ltr;

    /**
     * Volume of boiler (hot water warmer)
     */
    int boiler_ltr;

    /**
     * heater inner pipes volume in litters
     */
    int heater_core_ltr;

    /**
     * pipes volume in litters
     */
    int pipes_and_radiators_ltr;
};

/**
 * Config for working with two warmers pumps
 */
struct TwoPumpsSettings {

    bool enabled;

    /**
     * switch on second pump on temperature between input and output flow reaches this value
     */
    float start_on_delta_temperature_between_input_and_output;

    /**
     * switch on second pump on temperature between input and output flow drops to this value
     */
    float stop_on_delta_temperature_between_input_and_output;

    /**
     * switch on second pump on temperature between input flow and core reaches this value
     */
    float start_on_delta_temperature_between_core_and_input;

    /**
     * switch off second pump on temperature between input flow and core drops yo this value
     */
    float stop_on_delta_temperature_between_core_and_input;

    /**
     * weather flow sensor installed and connected or now
     */
    bool flow_senser_installed;

    /**
     * how many litters flows through sensor to raise one tick. in litters
     */
    float volume_per_one_sensors_tick_litters;

    /**
     * will be used if flow sensor is absent
     */
    float first_pump_flow_litters_per_minute;

    /**
     * will be used if flow sensor is absent
     */
    float second_pump_flow_litters_per_minute;

};

struct WarmingSettings {
   /**
    * Value of EMA difference of core temperatures to go from the warming mode to the final cooling mode
    */
    float warming_to_cooling_DiffEMA;

    /**
     * how long must takes cooling for enter to FINAL COOLING mode
     */
    int go_to_cooling_mode_if_DEMA_less_tan_0_more_than_sec;
};

struct FinalCoolingSettings {
   /**
    * Core temperatures at which you need to switch to STAND_BY mode
    */
    float cooling_to_standBy_temperature;

    /**
     * how long must takes cooling for enter to STANDBY mode
     */
    int go_to_stanby_mode_if_DEMA_less_tan_0_more_than_sec;
};

struct StadbyCoolingSettings {
    /**
     * Start first pump when core temperature starts rising and reaches this value
     */
    // TODO rename in interface
    float start_warming_cycle_on_power;

    /**
     * min_delta_btw_core_input_to_start_pump
     */
    float min_delta_btw_core_and_input_to_start_pumps;

    /**
     *
     */
    float start_pumps_temperature;

    /**
     * when core cooled by this value pumps will be stopped
     */
    float delta_btw_start_and_core_to_stop_pumps;

};

struct TemperatureSettings {
    /**
     * core critical temperature after reaching turn on alarm if all action to prevent it completed
     */
    float core_critical;

    /**
     * core overheat temperature after reaching action to fast cooling
     */
    float core_overheat;

    /**
     * the working core temperature
     */
    double core_target;

    /**
     * temperature at which to inform to refuel warmer
     */
    float core_low;

    /**
     * expected temperature of the thermal accumulator
     */
    float thermal_accumulator_target;

    /**
     * Temperature to start PID mode
     */
    float start_pid_temperature;

    /**
     * EMA for difference of core temperature
     */
    float core_temp_diff_EMA;

    /**
     * DEMA for difference of core power
     */
    float core_power_diff_EMA;

    /**
     * EMA for core power
     */
    float core_power_EMA;
};

struct ServoHardwareSettings {
    int min_impulse_length_us;
    int max_impulse_length_us;
    float total_degrees;

    int working_min_angle;
    int working_max_angle;
};

struct ServosHardwareSettings {
    ServoHardwareSettings oxygen_servo_settings;
    ServoHardwareSettings smoke_servo_settings;
    ServoHardwareSettings upper_door_servo_settings;
};

struct HeaterSettings {

    long scan_interval_ms;

    VolumeCapacitiesSetting capacities_setting;

    PidSettings oxygen_pid;

    TwoPumpsSettings two_pumps_settings;

    ServosHardwareSettings servos_hardware_settings;

    TemperatureSettings temperatureSettings;

    WarmingSettings warmingSettings;

    FinalCoolingSettings finalCoolingSettings;

    StadbyCoolingSettings stadbyCoolingSettings;
};

#endif //BASE_ESP8266_MQTT_HEATERSETTINGS_H
