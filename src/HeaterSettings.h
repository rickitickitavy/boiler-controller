//
// Created by dsporykhin on 31.03.22.
//

#ifndef BASE_ESP8266_MQTT_HEATERSETTINGS_H
#define BASE_ESP8266_MQTT_HEATERSETTINGS_H


struct PidSettings {
    float p;
    float i;

    float d;

    float max_i;
    float min_i;

    float power_to_switch_to_warming_mode;
    float oxygen_door_val_to_warming_mode;
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

struct CoolingByPumpsSettings{
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
     * Value of EMA power to switch to the PID mode. or temperature reaches start_pid_temperature
     */
    float target_power_to_switch_to_the_PID_mode;

    /**
     * Temperature to start PID mode. or power reaches target_power_to_switch_to_the_PID_mode
    */
    float start_pid_temperature;


    /**
     * how long must takes warming for switch to PID mode. If warming takes more time then you must
     * switch to the final cooling mode
     */
    int time_to_reach_target_power_sec;
};

struct FinalCoolingSettings {
    /**
     * Core temperatures at which you need to switch to STAND_BY mode
     */
    float max_temperature_to_switch_to_standBy;

    /**
     * how long time power must be less than standby_mode.start_warming_cycle_on_power
     * to switch to the STANDBY mode
     */
    int delay_to_switch_to_standby_mode_sec;

    /**
     *  switch to warming mode when power reaches this value
     */
    float power_to_switch_to_warming_mode;
};

struct StadbyCoolingSettings {
    /**
     * Start first pump when core temperature starts rising and reaches this value
     */
    // TODO rename in interface
    float start_warming_cycle_on_power;

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
    float core_target;

    /**
     * temperature at which to inform to refuel warmer
     */
    float core_power_when_need_to_refuel;

    /**
     * expected temperature of the thermal accumulator
     */
    float thermal_accumulator_target;

    /**
     * EMA for difference of core temperature
     */
    int core_temp_diff_EMA;

    /**
     * DEMA for difference of core power
     */
    int core_power_diff_EMA;

    /**
     * EMA for core power
     */
    int core_power_EMA;

    /**
     * interval for calc SMA temperature
     */
    int SMA_temperature_period_sec;
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

struct ModellerSettings{
    float max_core_power;
    float core_power_grow_angle_step;
    int max_power_cycles;
    float core_energy_transmitting_coef;
    int skip_first_N_cycles;
    int ema_doors_reactions;
    float radiators_base_power;
    float radiators_home_temperature;
    float radiators_base_temperature;
    float radiators_efficiensy_coef_pow;
    int radiator_stop_after_cycle;
    float radiators_flow_lpm;
    float additional_core_doors_coef;
    int length_of_modeling_cycles;
    bool logging_modeller_info;
    int power_fade_out_steps;
};

struct HeaterSettings {

    int scan_interval_ms;

    VolumeCapacitiesSetting capacities_setting;

    PidSettings oxygen_pid;

    TwoPumpsSettings two_pumps_settings;

    ServosHardwareSettings servos_hardware_settings;

    TemperatureSettings temperatureSettings;

    WarmingSettings warmingSettings;

    FinalCoolingSettings finalCoolingSettings;

    StadbyCoolingSettings stadbyCoolingSettings;

    CoolingByPumpsSettings coolingByPumpsSettings;

    ModellerSettings modellerSettings;
};

#endif //BASE_ESP8266_MQTT_HEATERSETTINGS_H
