//
// Created by dsporykhin on 31.03.22.
//

#ifndef BASE_ESP8266_MQTT_HEATERSETTINGS_H
#define BASE_ESP8266_MQTT_HEATERSETTINGS_H

#endif //BASE_ESP8266_MQTT_HEATERSETTINGS_H

struct PidSettings{
    double p;
    double i;
    double d;
    double max_i;
    /**
     * minimum of output value of PID
     */
    double min_absolute_output_value_prcnt;

    /**
     * diapason of vales of PID output
     */
    double output_dynamic_diapason_prcnt;
};

struct VolumeCapacitiesSetting{
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
struct TwoPumpsSettings{

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

};

struct TemperatureSettings{
    /**
     * core critical temperature after reaching turn on alarm if all action to prevent it completed
     */
    float core_critical;

    /**
     * the working core temperature
     */
    float core_target;

    /**
     * temperature at which to inform to refuel warmer
     */
    float core_low;

    /**
     * expected temperature of the thermal accumulator
     */
    float thermal_accumulator_target;
};

struct ServoHardwareSettings{
    int min_impulse_length_us;
    int max_impulseLength_us;
    float total_degrees;

    int working_min_available_angle;
    int working_max_available_degrees;
};

struct ServosHardwareSettings{
    ServoHardwareSettings oxygen_servo_settings;
    ServoHardwareSettings smoke_servo_settings;
    ServoHardwareSettings upper_door_servo_settings;
};

struct HeaterSettings{

    long scan_interval_ms;

    /**
     * Start first pump when core temperature starts rising and reaches this value
     */
    float start_burn_cycle_on_temperature_up_to;

    VolumeCapacitiesSetting capacities_setting;

    PidSettings oxygen_pid;

    TwoPumpsSettings two_pumps_settings;

    ServosHardwareSettings servos_hardware_settings;
};