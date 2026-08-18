//
// Created by dsporykhin on 24.04.20.
//

#include <IPAddress.h>
#include "SettingsNavigator.h"
#include "Logger.h"
#include "Converter.h"

SettingsNavigator::SettingsNavigator(SettingsManager *settingsManager) {
    this->settingsManager = settingsManager;
    this->settings = settingsManager->getSettings();

    this->paramDescriptors[activeParamDescriptors++] = new ParamDescriptor("network>ssid", STRING, 5,
                                                                           63,
                                                                           (void *) &settings->network.ssid[0],
                                                                           (void *) &settings->network.ssid[0]);
    this->paramDescriptors[activeParamDescriptors++] = new ParamDescriptor("network>password", STRING, 5,
                                                                           63,
                                                                           (void *) &settings->network.password[0],
                                                                           (void *) &settings->network.password[0]);

    this->paramDescriptors[activeParamDescriptors++] = new ParamDescriptor("mqtt>server", STRING, 5,
                                                                           63,
                                                                           (void *) &settings->mqttServer[0],
                                                                           (void *) &settings->mqttServer[0]);
    this->paramDescriptors[activeParamDescriptors++] = new ParamDescriptor("mqtt>port", INTEGER, 30, 65534,
                                                                           (void *) &settings->mqttPort,
                                                                           (void *) &settings->mqttPort);
    this->paramDescriptors[activeParamDescriptors++] = new ParamDescriptor("mqtt>reconnectIntervalMs", INTEGER, 30,
                                                                           60000,
                                                                           (void *) &settings->mqttReconnectIntervalMs,
                                                                           (void *) &settings->mqttReconnectIntervalMs);
    this->paramDescriptors[activeParamDescriptors++] = new ParamDescriptor("mqtt>deviceName", STRING, 2,
                                                                           31,
                                                                           (void *) &settings->mqttDeviceName[0],
                                                                           (void *) &settings->mqttDeviceName[0]);
    this->paramDescriptors[activeParamDescriptors++] = new ParamDescriptor("mqtt>serverBornTopic", STRING, 2,
                                                                           63,
                                                                           (void *) &settings->mqttServerBornTopic[0],
                                                                           (void *) &settings->mqttServerBornTopic[0]);
    this->paramDescriptors[activeParamDescriptors++] = new ParamDescriptor("device>stateOutgoingTopicPrefix", STRING, 5,
                                                                           31,
                                                                           (void *) &settings->deviceStateOutgoingTopicPrefix[0],
                                                                           (void *) &settings->deviceStateOutgoingTopicPrefix[0]);
    this->paramDescriptors[activeParamDescriptors++] = new ParamDescriptor("device>iHaveBornTopic", STRING, 5,
                                                                           63,
                                                                           (void *) &settings->deviceIHaveBornTopic[0],
                                                                           (void *) &settings->deviceIHaveBornTopic[0]);
    this->paramDescriptors[activeParamDescriptors++] = new ParamDescriptor("device>incomingCommandTopicPrefix", STRING,
                                                                           5,
                                                                           31,
                                                                           (void *) &settings->deviceIncomingCommandTopicPrefix[0],
                                                                           (void *) &settings->deviceIncomingCommandTopicPrefix[0]);

    this->paramDescriptors[activeParamDescriptors++] = new ParamDescriptor("device>mqttInputToolTopic", STRING, 5,
                                                                           31,
                                                                           (void *) &settings->mqttInputToolTopic[0],
                                                                           (void *) &settings->mqttInputToolTopic[0]);
    this->paramDescriptors[activeParamDescriptors++] = new ParamDescriptor("device>mqttOutputToolTopic", STRING, 5,
                                                                           31,
                                                                           (void *) &settings->mqttOutputToolTopic[0],
                                                                           (void *) &settings->mqttOutputToolTopic[0]);
    this->paramDescriptors[activeParamDescriptors++] = new ParamDescriptor("device>defaultSwitcherState", UCHAR, 0,
                                                                           255,
                                                                           (void *) &settings->defaultSwitcherState,
                                                                           (void *) &settings->defaultSwitcherState);
    this->paramDescriptors[activeParamDescriptors++] = new ParamDescriptor("device>dimmerPwmValue", INTEGER, 0,
                                                                           1024,
                                                                           (void *) &settings->dimmerPwmValue,
                                                                           (void *) &settings->dimmerPwmValue);
    this->paramDescriptors[activeParamDescriptors++] = new ParamDescriptor("sensors>heater>core", SENSORS_ADDR_SIZE,
                                                                           (void *) &settings->ds18D20Addresses[T_SENS_INDEX_CORE],
                                                                           (void *) &settings->ds18D20Addresses[T_SENS_INDEX_CORE]);
    this->paramDescriptors[activeParamDescriptors++] = new ParamDescriptor("sensors>heater>output_flow",
                                                                           SENSORS_ADDR_SIZE,
                                                                           (void *) &settings->ds18D20Addresses[
                                                                                   SENSORS_ADDR_SIZE *
                                                                                   T_SENS_INDEX_OUTPUT_FLOW],
                                                                           (void *) &settings->ds18D20Addresses[
                                                                                   SENSORS_ADDR_SIZE *
                                                                                   T_SENS_INDEX_OUTPUT_FLOW]);
    this->paramDescriptors[activeParamDescriptors++] = new ParamDescriptor("sensors>heater>input_flow",
                                                                           SENSORS_ADDR_SIZE,
                                                                           (void *) &settings->ds18D20Addresses[
                                                                                   SENSORS_ADDR_SIZE *
                                                                                   T_SENS_INDEX_INPUT_FLOW],
                                                                           (void *) &settings->ds18D20Addresses[
                                                                                   SENSORS_ADDR_SIZE *
                                                                                   T_SENS_INDEX_INPUT_FLOW]);

    this->paramDescriptors[activeParamDescriptors++] = new ParamDescriptor("sensors>termoaccumulator>top",
                                                                           SENSORS_ADDR_SIZE,
                                                                           (void *) &settings->ds18D20Addresses[
                                                                                   SENSORS_ADDR_SIZE *
                                                                                   T_SENS_INDEX_ACC_TOP],
                                                                           (void *) &settings->ds18D20Addresses[
                                                                                   SENSORS_ADDR_SIZE *
                                                                                   T_SENS_INDEX_ACC_TOP]);
    this->paramDescriptors[activeParamDescriptors++] = new ParamDescriptor("sensors>termoaccumulator>middle_hi",
                                                                           SENSORS_ADDR_SIZE,
                                                                           (void *) &settings->ds18D20Addresses[
                                                                                   SENSORS_ADDR_SIZE *
                                                                                   T_SENS_INDEX_ACC_MID_HI],
                                                                           (void *) &settings->ds18D20Addresses[
                                                                                   SENSORS_ADDR_SIZE *
                                                                                   T_SENS_INDEX_ACC_MID_HI]);
    this->paramDescriptors[activeParamDescriptors++] = new ParamDescriptor("sensors>termoaccumulator>middle_low",
                                                                           SENSORS_ADDR_SIZE,
                                                                           (void *) &settings->ds18D20Addresses[
                                                                                   SENSORS_ADDR_SIZE *
                                                                                   T_SENS_INDEX_ACC_MID_LO],
                                                                           (void *) &settings->ds18D20Addresses[
                                                                                   SENSORS_ADDR_SIZE *
                                                                                   T_SENS_INDEX_ACC_MID_LO]);
    this->paramDescriptors[activeParamDescriptors++] = new ParamDescriptor("sensors>termoaccumulator>bottom",
                                                                           SENSORS_ADDR_SIZE,
                                                                           (void *) &settings->ds18D20Addresses[
                                                                                   SENSORS_ADDR_SIZE *
                                                                                   T_SENS_INDEX_ACC_BOTTOM],
                                                                           (void *) &settings->ds18D20Addresses[
                                                                                   SENSORS_ADDR_SIZE *
                                                                                   T_SENS_INDEX_ACC_BOTTOM]);
    this->paramDescriptors[activeParamDescriptors++] = new ParamDescriptor("sensors>forward>temperature",
                                                                           SENSORS_ADDR_SIZE,
                                                                           (void *) &settings->ds18D20Addresses[
                                                                                   SENSORS_ADDR_SIZE *
                                                                                   T_SENS_INDEX_FORWARD_FLOW],
                                                                           (void *) &settings->ds18D20Addresses[
                                                                                   SENSORS_ADDR_SIZE *
                                                                                   T_SENS_INDEX_FORWARD_FLOW]);
    this->paramDescriptors[activeParamDescriptors++] = new ParamDescriptor("sensors>backward>temperature",
                                                                           SENSORS_ADDR_SIZE,
                                                                           (void *) &settings->ds18D20Addresses[
                                                                                   SENSORS_ADDR_SIZE *
                                                                                   T_SENS_INDEX_BACKWARD_FLOW],
                                                                           (void *) &settings->ds18D20Addresses[
                                                                                   SENSORS_ADDR_SIZE *
                                                                                   T_SENS_INDEX_BACKWARD_FLOW]);
    this->paramDescriptors[activeParamDescriptors++] = new ParamDescriptor("sensors>internal>temperature",
                                                                           SENSORS_ADDR_SIZE,
                                                                           (void *) &settings->ds18D20Addresses[
                                                                                   SENSORS_ADDR_SIZE *
                                                                                   T_SENS_INDEX_INTERNAL],
                                                                           (void *) &settings->ds18D20Addresses[
                                                                                   SENSORS_ADDR_SIZE *
                                                                                           T_SENS_INDEX_INTERNAL]);
    this->paramDescriptors[activeParamDescriptors++] = new ParamDescriptor("sensors>back>contour2", SENSORS_ADDR_SIZE,
                                                                           (void *) &settings->ds18D20Addresses[
                                                                                   SENSORS_ADDR_SIZE * 9],
                                                                           (void *) &settings->ds18D20Addresses[
                                                                                   SENSORS_ADDR_SIZE * 9]);
    this->paramDescriptors[activeParamDescriptors++] = new ParamDescriptor("sensors>back>contour3", SENSORS_ADDR_SIZE,
                                                                           (void *) &settings->ds18D20Addresses[
                                                                                   SENSORS_ADDR_SIZE * 10],
                                                                           (void *) &settings->ds18D20Addresses[
                                                                                   SENSORS_ADDR_SIZE * 10]);
    this->paramDescriptors[activeParamDescriptors++] = new ParamDescriptor("sensors>send_data_to_mqtt_interval_ms",
                                                                           INTEGER, 3000,
                                                                           120000,
                                                                           (void *) &settings->send_data_to_mqtt_interval_ms,
                                                                           (void *) &settings->send_data_to_mqtt_interval_ms);


    // ================= ALL HEATER SETTINGS
    this->paramDescriptors[activeParamDescriptors++] = new ParamDescriptor("heater>scan_interval_ms", INTEGER, 1000,
                                                                           120000,
                                                                           (void *) &settings->heaterSettings.scan_interval_ms,
                                                                           (void *) &settings->heaterSettings.scan_interval_ms);

    // volume capacities
    this->paramDescriptors[activeParamDescriptors++] = new ParamDescriptor("heater>capacities>heater_core_ltr", INTEGER,
                                                                           1,
                                                                           150,
                                                                           (void *) &settings->heaterSettings.capacities_setting.heater_core_ltr,
                                                                           (void *) &settings->heaterSettings.capacities_setting.heater_core_ltr);
    this->paramDescriptors[activeParamDescriptors++] = new ParamDescriptor("heater>capacities>accumulator_ltr", INTEGER,
                                                                           0,
                                                                           3000,
                                                                           (void *) &settings->heaterSettings.capacities_setting.accumulator_ltr,
                                                                           (void *) &settings->heaterSettings.capacities_setting.accumulator_ltr);
    this->paramDescriptors[activeParamDescriptors++] = new ParamDescriptor("heater>capacities>boiler_ltr", INTEGER, 1,
                                                                           500,
                                                                           (void *) &settings->heaterSettings.capacities_setting.boiler_ltr,
                                                                           (void *) &settings->heaterSettings.capacities_setting.boiler_ltr);
    this->paramDescriptors[activeParamDescriptors++] = new ParamDescriptor("heater>capacities>pipes_and_radiators_ltr",
                                                                           INTEGER, 1,
                                                                           1000,
                                                                           (void *) &settings->heaterSettings.capacities_setting.pipes_and_radiators_ltr,
                                                                           (void *) &settings->heaterSettings.capacities_setting.pipes_and_radiators_ltr);

    // PID
    this->paramDescriptors[activeParamDescriptors++] = new ParamDescriptor("heater>pid>smoke_door_value_prc", INTEGER, 40,
                                                                           100,
                                                                           (void *) &settings->heaterSettings.burning_settings.smoke_door_value_prcnt,
                                                                           (void *) &settings->heaterSettings.burning_settings.smoke_door_value_prcnt);
    this->paramDescriptors[activeParamDescriptors++] = new ParamDescriptor("heater>pid>upper_door_value_prc", INTEGER, 40,
                                                                           100,
                                                                           (void *) &settings->heaterSettings.burning_settings.upper_door_value_prcnt,
                                                                           (void *) &settings->heaterSettings.burning_settings.upper_door_value_prcnt);
    this->paramDescriptors[activeParamDescriptors++] = new ParamDescriptor("heater>pid>p", FLOAT, 1,
                                                                           1000,
                                                                           (void *) &settings->heaterSettings.burning_settings.p,
                                                                           (void *) &settings->heaterSettings.burning_settings.p);
    this->paramDescriptors[activeParamDescriptors++] = new ParamDescriptor("heater>pid>i", FLOAT, 0.01,
                                                                           100,
                                                                           (void *) &settings->heaterSettings.burning_settings.i,
                                                                           (void *) &settings->heaterSettings.burning_settings.i);
    this->paramDescriptors[activeParamDescriptors++] = new ParamDescriptor("heater>pid>d", FLOAT, 0.01,
                                                                           1000,
                                                                           (void *) &settings->heaterSettings.burning_settings.d,
                                                                           (void *) &settings->heaterSettings.burning_settings.d);
    this->paramDescriptors[activeParamDescriptors++] = new ParamDescriptor("heater>pid>max_i", FLOAT, 1,
                                                                           100,
                                                                           (void *) &settings->heaterSettings.burning_settings.max_i,
                                                                           (void *) &settings->heaterSettings.burning_settings.max_i);
    this->paramDescriptors[activeParamDescriptors++] = new ParamDescriptor("heater>pid>min_i", FLOAT, -100,
                                                                           -1,
                                                                           (void *) &settings->heaterSettings.burning_settings.min_i,
                                                                           (void *) &settings->heaterSettings.burning_settings.min_i);
    this->paramDescriptors[activeParamDescriptors++] = new ParamDescriptor("heater>pid>pwr_to_sw_to_warm", FLOAT, 5000,
                                                                           40000,
                                                                           (void *) &settings->heaterSettings.burning_settings.power_to_switch_to_warming_mode,
                                                                           (void *) &settings->heaterSettings.burning_settings.power_to_switch_to_warming_mode);
    this->paramDescriptors[activeParamDescriptors++] = new ParamDescriptor("heater>pid>oxy_door_to_sw_warm", FLOAT, 2,
                                                                           90,
                                                                           (void *) &settings->heaterSettings.burning_settings.oxygen_door_val_to_warming_mode,
                                                                           (void *) &settings->heaterSettings.burning_settings.oxygen_door_val_to_warming_mode);
    // two pumps
    this->paramDescriptors[activeParamDescriptors++] = new ParamDescriptor("heater>2pumps>enabled", CHECK_BOX, 0,
                                                                           1,
                                                                           (void *) &settings->heaterSettings.two_pumps_settings.enabled,
                                                                           (void *) &settings->heaterSettings.two_pumps_settings.enabled);
    this->paramDescriptors[activeParamDescriptors++] = new ParamDescriptor(
            "heater>2pumps>start_on_delta_btw_core_input", FLOAT, 10,
            40,
            (void *) &settings->heaterSettings.two_pumps_settings.start_on_delta_temperature_between_core_and_input,
            (void *) &settings->heaterSettings.two_pumps_settings.start_on_delta_temperature_between_core_and_input);
    this->paramDescriptors[activeParamDescriptors++] = new ParamDescriptor("heater>2pumps>stop_on_delta_btw_core_input",
                                                                           FLOAT, 7,
                                                                           37,
                                                                           (void *) &settings->heaterSettings.two_pumps_settings.stop_on_delta_temperature_between_core_and_input,
                                                                           (void *) &settings->heaterSettings.two_pumps_settings.stop_on_delta_temperature_between_core_and_input);
    this->paramDescriptors[activeParamDescriptors++] = new ParamDescriptor(
            "heater>2pumps>start_on_delta_btw_in_and_out", FLOAT, 10,
            25,
            (void *) &settings->heaterSettings.two_pumps_settings.start_on_delta_temperature_between_input_and_output,
            (void *) &settings->heaterSettings.two_pumps_settings.start_on_delta_temperature_between_input_and_output);
    this->paramDescriptors[activeParamDescriptors++] = new ParamDescriptor("heater>2pumps>stop_on_delta_btw_in_and_out",
                                                                           FLOAT, 7,
                                                                           22,
                                                                           (void *) &settings->heaterSettings.two_pumps_settings.stop_on_delta_temperature_between_input_and_output,
                                                                           (void *) &settings->heaterSettings.two_pumps_settings.stop_on_delta_temperature_between_input_and_output);
    this->paramDescriptors[activeParamDescriptors++] = new ParamDescriptor("heater>2pumps>f_sens_installed", CHECK_BOX,
                                                                           0,
                                                                           1,
                                                                           (void *) &settings->heaterSettings.two_pumps_settings.flow_senser_installed,
                                                                           (void *) &settings->heaterSettings.two_pumps_settings.flow_senser_installed);
    this->paramDescriptors[activeParamDescriptors++] = new ParamDescriptor("heater>2pumps>flow_sens_ticks_per_lt", FLOAT,
                                                                           1,
                                                                           50,
                                                                           (void *) &settings->heaterSettings.two_pumps_settings.flow_sensor_ticks_per_litters,
                                                                           (void *) &settings->heaterSettings.two_pumps_settings.flow_sensor_ticks_per_litters);
    this->paramDescriptors[activeParamDescriptors++] = new ParamDescriptor("heater>2pumps>1st_pump_flow_lpm", FLOAT, 4,
                                                                           40,
                                                                           (void *) &settings->heaterSettings.two_pumps_settings.first_pump_flow_litters_per_minute,
                                                                           (void *) &settings->heaterSettings.two_pumps_settings.first_pump_flow_litters_per_minute);
    this->paramDescriptors[activeParamDescriptors++] = new ParamDescriptor("heater>2pumps>2st_pump_flow_lpm", FLOAT, 4,
                                                                           40,
                                                                           (void *) &settings->heaterSettings.two_pumps_settings.second_pump_flow_litters_per_minute,
                                                                           (void *) &settings->heaterSettings.two_pumps_settings.second_pump_flow_litters_per_minute);

    // *************** SERVOS
    // oxygen
    this->paramDescriptors[activeParamDescriptors++] = new ParamDescriptor("heater>servos>oxygen>min_impulse_length_us",
                                                                           INTEGER, 600,
                                                                           1200,
                                                                           (void *) &settings->heaterSettings.servos_hardware_settings.oxygen_servo_settings.min_impulse_length_us,
                                                                           (void *) &settings->heaterSettings.servos_hardware_settings.oxygen_servo_settings.min_impulse_length_us);
    this->paramDescriptors[activeParamDescriptors++] = new ParamDescriptor("heater>servos>oxygen>max_impulse_length_us",
                                                                           INTEGER, 1200,
                                                                           3600,
                                                                           (void *) &settings->heaterSettings.servos_hardware_settings.oxygen_servo_settings.max_impulse_length_us,
                                                                           (void *) &settings->heaterSettings.servos_hardware_settings.oxygen_servo_settings.max_impulse_length_us);
    this->paramDescriptors[activeParamDescriptors++] = new ParamDescriptor("heater>servos>oxygen>total_degrees", FLOAT,
                                                                           90,
                                                                           360,
                                                                           (void *) &settings->heaterSettings.servos_hardware_settings.oxygen_servo_settings.total_degrees,
                                                                           (void *) &settings->heaterSettings.servos_hardware_settings.oxygen_servo_settings.total_degrees);
    this->paramDescriptors[activeParamDescriptors++] = new ParamDescriptor("heater>servos>oxygen>working_min_angle",
                                                                           INTEGER, 0,
                                                                           180,
                                                                           (void *) &settings->heaterSettings.servos_hardware_settings.oxygen_servo_settings.working_min_angle,
                                                                           (void *) &settings->heaterSettings.servos_hardware_settings.oxygen_servo_settings.working_min_angle);
    this->paramDescriptors[activeParamDescriptors++] = new ParamDescriptor("heater>servos>oxygen>working_max_angle",
                                                                           INTEGER, 1,
                                                                           360,
                                                                           (void *) &settings->heaterSettings.servos_hardware_settings.oxygen_servo_settings.working_max_angle,
                                                                           (void *) &settings->heaterSettings.servos_hardware_settings.oxygen_servo_settings.working_max_angle);
    // smoke
    this->paramDescriptors[activeParamDescriptors++] = new ParamDescriptor("heater>servos>smoke>min_impulse_length_us",
                                                                           INTEGER, 600,
                                                                           1200,
                                                                           (void *) &settings->heaterSettings.servos_hardware_settings.smoke_servo_settings.min_impulse_length_us,
                                                                           (void *) &settings->heaterSettings.servos_hardware_settings.smoke_servo_settings.min_impulse_length_us);
    this->paramDescriptors[activeParamDescriptors++] = new ParamDescriptor("heater>servos>smoke>max_impulse_length_us",
                                                                           INTEGER, 1200,
                                                                           3600,
                                                                           (void *) &settings->heaterSettings.servos_hardware_settings.smoke_servo_settings.max_impulse_length_us,
                                                                           (void *) &settings->heaterSettings.servos_hardware_settings.smoke_servo_settings.max_impulse_length_us);
    this->paramDescriptors[activeParamDescriptors++] = new ParamDescriptor("heater>servos>smoke>total_degrees", FLOAT,
                                                                           90,
                                                                           360,
                                                                           (void *) &settings->heaterSettings.servos_hardware_settings.smoke_servo_settings.total_degrees,
                                                                           (void *) &settings->heaterSettings.servos_hardware_settings.smoke_servo_settings.total_degrees);
    this->paramDescriptors[activeParamDescriptors++] = new ParamDescriptor("heater>servos>smoke>working_min_angle",
                                                                           INTEGER, 0,
                                                                           180,
                                                                           (void *) &settings->heaterSettings.servos_hardware_settings.smoke_servo_settings.working_min_angle,
                                                                           (void *) &settings->heaterSettings.servos_hardware_settings.smoke_servo_settings.working_min_angle);
    this->paramDescriptors[activeParamDescriptors++] = new ParamDescriptor("heater>servos>smoke>working_max_angle",
                                                                           INTEGER, 1,
                                                                           360,
                                                                           (void *) &settings->heaterSettings.servos_hardware_settings.smoke_servo_settings.working_max_angle,
                                                                           (void *) &settings->heaterSettings.servos_hardware_settings.smoke_servo_settings.working_max_angle);
    // upper
    this->paramDescriptors[activeParamDescriptors++] = new ParamDescriptor("heater>servos>upper>min_impulse_length_us",
                                                                           INTEGER, 600,
                                                                           1200,
                                                                           (void *) &settings->heaterSettings.servos_hardware_settings.upper_door_servo_settings.min_impulse_length_us,
                                                                           (void *) &settings->heaterSettings.servos_hardware_settings.upper_door_servo_settings.min_impulse_length_us);
    this->paramDescriptors[activeParamDescriptors++] = new ParamDescriptor("heater>servos>upper>max_impulse_length_us",
                                                                           INTEGER, 1200,
                                                                           3600,
                                                                           (void *) &settings->heaterSettings.servos_hardware_settings.upper_door_servo_settings.max_impulse_length_us,
                                                                           (void *) &settings->heaterSettings.servos_hardware_settings.upper_door_servo_settings.max_impulse_length_us);
    this->paramDescriptors[activeParamDescriptors++] = new ParamDescriptor("heater>servos>upper>total_degrees", FLOAT,
                                                                           90,
                                                                           360,
                                                                           (void *) &settings->heaterSettings.servos_hardware_settings.upper_door_servo_settings.total_degrees,
                                                                           (void *) &settings->heaterSettings.servos_hardware_settings.upper_door_servo_settings.total_degrees);
    this->paramDescriptors[activeParamDescriptors++] = new ParamDescriptor("heater>servos>upper>working_min_angle",
                                                                           INTEGER, 0,
                                                                           180,
                                                                           (void *) &settings->heaterSettings.servos_hardware_settings.upper_door_servo_settings.working_min_angle,
                                                                           (void *) &settings->heaterSettings.servos_hardware_settings.upper_door_servo_settings.working_min_angle);
    this->paramDescriptors[activeParamDescriptors++] = new ParamDescriptor("heater>servos>upper>working_max_angle",
                                                                           INTEGER, 1,
                                                                           360,
                                                                           (void *) &settings->heaterSettings.servos_hardware_settings.upper_door_servo_settings.working_max_angle,
                                                                           (void *) &settings->heaterSettings.servos_hardware_settings.upper_door_servo_settings.working_max_angle);

    // Settings for the upper doors control
    this->paramDescriptors[activeParamDescriptors++] = new ParamDescriptor("heater>interval_for_calc_power_acc_sec",
                                                                           INTEGER, 30,
                                                                           300,
                                                                           (void *) &settings->interval_for_calc_power_different_sec,
                                                                           (void *) &settings->interval_for_calc_power_different_sec);
    this->paramDescriptors[activeParamDescriptors++] = new ParamDescriptor("heater>core_power_to_close_upper_door_W",
                                                                           INTEGER, 15000,
                                                                           40000,
                                                                           (void *) &settings->core_power_to_close_upper_door_if_ICPD_W,
                                                                           (void *) &settings->core_power_to_close_upper_door_if_ICPD_W);


    // Temperature settings
    this->paramDescriptors[activeParamDescriptors++] = new ParamDescriptor("heater>temperature>core_pwr_to_refuel",
                                                                           FLOAT, 5000,
                                                                           25000,
                                                                           (void *) &settings->heaterSettings.temperatureSettings.core_power_when_need_to_refuel,
                                                                           (void *) &settings->heaterSettings.temperatureSettings.core_power_when_need_to_refuel);
    this->paramDescriptors[activeParamDescriptors++] = new ParamDescriptor("heater>temperature>core_target", FLOAT, 70,
                                                                           95,
                                                                           (void *) &settings->heaterSettings.temperatureSettings.core_target,
                                                                           (void *) &settings->heaterSettings.temperatureSettings.core_target);
    this->paramDescriptors[activeParamDescriptors++] = new ParamDescriptor("heater>temperature>core_overheat", FLOAT,
                                                                           85,
                                                                           99,
                                                                           (void *) &settings->heaterSettings.temperatureSettings.core_overheat,
                                                                           (void *) &settings->heaterSettings.temperatureSettings.core_overheat);
    this->paramDescriptors[activeParamDescriptors++] = new ParamDescriptor("heater>temperature>core_critical", FLOAT,
                                                                           90,
                                                                           100,
                                                                           (void *) &settings->heaterSettings.temperatureSettings.core_critical,
                                                                           (void *) &settings->heaterSettings.temperatureSettings.core_critical);
    this->paramDescriptors[activeParamDescriptors++] = new ParamDescriptor("heater>temperature>term_accumulator_target",
                                                                           FLOAT, 70,
                                                                           90,
                                                                           (void *) &settings->heaterSettings.temperatureSettings.thermal_accumulator_target,
                                                                           (void *) &settings->heaterSettings.temperatureSettings.thermal_accumulator_target);
    this->paramDescriptors[activeParamDescriptors++] = new ParamDescriptor("heater>temperature>tempr_sma_period_sec",
                                                                           INTEGER, 10,
                                                                           600,
                                                                           (void *) &settings->heaterSettings.temperatureSettings.SMA_temperature_period_sec,
                                                                           (void *) &settings->heaterSettings.temperatureSettings.SMA_temperature_period_sec);
    this->paramDescriptors[activeParamDescriptors++] = new ParamDescriptor("heater>temperature>core_power_ema", INTEGER,
                                                                           1,
                                                                           50,
                                                                           (void *) &settings->heaterSettings.temperatureSettings.core_power_EMA,
                                                                           (void *) &settings->heaterSettings.temperatureSettings.core_power_EMA);
    this->paramDescriptors[activeParamDescriptors++] = new ParamDescriptor("heater>temperature>core_d_power_ema",
                                                                           INTEGER, 1,
                                                                           50,
                                                                           (void *) &settings->heaterSettings.temperatureSettings.core_power_diff_EMA,
                                                                           (void *) &settings->heaterSettings.temperatureSettings.core_power_diff_EMA);
    this->paramDescriptors[activeParamDescriptors++] = new ParamDescriptor("heater>temperature>core_d_tempr_ema",
                                                                           INTEGER, 1,
                                                                           150,
                                                                           (void *) &settings->heaterSettings.temperatureSettings.core_temp_diff_EMA,
                                                                           (void *) &settings->heaterSettings.temperatureSettings.core_temp_diff_EMA);

    // STANBY SETTING
    this->paramDescriptors[activeParamDescriptors++] = new ParamDescriptor("heater>stanby>start_warm_on_power", FLOAT,
                                                                           200,
                                                                           7000,
                                                                           (void *) &settings->heaterSettings.stadbyCoolingSettings.start_warming_cycle_on_power,
                                                                           (void *) &settings->heaterSettings.stadbyCoolingSettings.start_warming_cycle_on_power);
    // WARMING SETTING
    this->paramDescriptors[activeParamDescriptors++] = new ParamDescriptor("heater>warming>pwr_to_sw_to_PID", FLOAT,
                                                                           3000,
                                                                           20000,
                                                                           (void *) &settings->heaterSettings.warming_settings.target_power_to_switch_to_the_PID_mode,
                                                                           (void *) &settings->heaterSettings.warming_settings.target_power_to_switch_to_the_PID_mode);
    this->paramDescriptors[activeParamDescriptors++] = new ParamDescriptor("heater>warming>core_tempr_to_sw", FLOAT,
                                                                           50,
                                                                           85,
                                                                           (void *) &settings->heaterSettings.warming_settings.start_pid_temperature,
                                                                           (void *) &settings->heaterSettings.warming_settings.start_pid_temperature);
    this->paramDescriptors[activeParamDescriptors++] = new ParamDescriptor("heater>warming>time_to_reach_pwr_sec",
                                                                           INTEGER,
                                                                           200,
                                                                           3600,
                                                                           (void *) &settings->heaterSettings.warming_settings.time_to_reach_target_power_sec,
                                                                           (void *) &settings->heaterSettings.warming_settings.time_to_reach_target_power_sec);
    this->paramDescriptors[activeParamDescriptors++] = new ParamDescriptor("heater>warming>smoke_door_value_prc",
                                                                           INTEGER,
                                                                           50,
                                                                           100,
                                                                           (void *) &settings->heaterSettings.warming_settings.smoke_door_value_prcnt,
                                                                           (void *) &settings->heaterSettings.warming_settings.smoke_door_value_prcnt);
    this->paramDescriptors[activeParamDescriptors++] = new ParamDescriptor("heater>warming>upper_door_value_prc",
                                                                           INTEGER,
                                                                           40,
                                                                           100,
                                                                           (void *) &settings->heaterSettings.warming_settings.upper_door_value_prcnt,
                                                                           (void *) &settings->heaterSettings.warming_settings.upper_door_value_prcnt);
    // FINAL COOLING SETTINGS
    this->paramDescriptors[activeParamDescriptors++] = new ParamDescriptor("heater>final_cool>tempr_to_sw_to_stby",
                                                                           FLOAT, 40,
                                                                           80,
                                                                           (void *) &settings->heaterSettings.finalCoolingSettings.max_temperature_to_switch_to_standBy,
                                                                           (void *) &settings->heaterSettings.finalCoolingSettings.max_temperature_to_switch_to_standBy);
    this->paramDescriptors[activeParamDescriptors++] = new ParamDescriptor("heater>final_cool>start_on_d_btw_core_inp",
                                                                           FLOAT, 3000,
                                                                           20000,
                                                                           (void *) &settings->heaterSettings.finalCoolingSettings.power_to_switch_to_warming_mode,
                                                                           (void *) &settings->heaterSettings.finalCoolingSettings.power_to_switch_to_warming_mode);
    this->paramDescriptors[activeParamDescriptors++] = new ParamDescriptor("heater>final_cool>delay_to_sw_to_stnby",
                                                                           INTEGER, 60,
                                                                           1800,
                                                                           (void *) &settings->heaterSettings.finalCoolingSettings.delay_to_switch_to_standby_mode_sec,
                                                                           (void *) &settings->heaterSettings.finalCoolingSettings.delay_to_switch_to_standby_mode_sec);
    // PUMPS COOLING SETTINGS
    this->paramDescriptors[activeParamDescriptors++] = new ParamDescriptor("heater>cool_pumps>start_on_d_btw_core_inp",
                                                                           FLOAT, 5,
                                                                           20,
                                                                           (void *) &settings->heaterSettings.coolingByPumpsSettings.min_delta_btw_core_and_input_to_start_pumps,
                                                                           (void *) &settings->heaterSettings.coolingByPumpsSettings.min_delta_btw_core_and_input_to_start_pumps);
    this->paramDescriptors[activeParamDescriptors++] = new ParamDescriptor("heater>cool_pumps>start_on_tempr", FLOAT,
                                                                           50,
                                                                           96,
                                                                           (void *) &settings->heaterSettings.coolingByPumpsSettings.start_pumps_temperature,
                                                                           (void *) &settings->heaterSettings.coolingByPumpsSettings.start_pumps_temperature);
    this->paramDescriptors[activeParamDescriptors++] = new ParamDescriptor("heater>cool_pumps>stop_on_d_btw_start_core",
                                                                           FLOAT, 3,
                                                                           20,
                                                                           (void *) &settings->heaterSettings.coolingByPumpsSettings.delta_btw_start_and_core_to_stop_pumps,
                                                                           (void *) &settings->heaterSettings.coolingByPumpsSettings.delta_btw_start_and_core_to_stop_pumps);
    // TELEMETRY
    this->paramDescriptors[activeParamDescriptors++] = new ParamDescriptor("telemetry>catalog_name", STRING, 3,
                                                                           63,
                                                                           (void *) &settings->telemetrySettings.catName[0],
                                                                           (void *) &settings->telemetrySettings.catName[0]);
    this->paramDescriptors[activeParamDescriptors++] = new ParamDescriptor("telemetry>max_file_size", INTEGER, 10240,
                                                                           8192 * 1024,
                                                                           (void *) &settings->telemetrySettings.max_file_size_bytes,
                                                                           (void *) &settings->telemetrySettings.max_file_size_bytes);
    this->paramDescriptors[activeParamDescriptors++] = new ParamDescriptor("telemetry>flush_interval_ms", INTEGER,
                                                                           10000,
                                                                           3600000,
                                                                           (void *) &settings->telemetrySettings.flush_interval_ms,
                                                                           (void *) &settings->telemetrySettings.flush_interval_ms);
    this->paramDescriptors[activeParamDescriptors++] = new ParamDescriptor("telemetry>flush_inteval_records", INTEGER,
                                                                           1,
                                                                           1000,
                                                                           (void *) &settings->telemetrySettings.flush_inteval_records,
                                                                           (void *) &settings->telemetrySettings.flush_inteval_records);
    this->paramDescriptors[activeParamDescriptors++] = new ParamDescriptor("telemetry>log_gebug_to_UART", CHECK_BOX,
                                                                           0,
                                                                           1,
                                                                           (void *) &settings->telemetrySettings.log_gebug_to_UART,
                                                                           (void *) &settings->telemetrySettings.log_gebug_to_UART);

    // MODELLER SETTINGS
    this->paramDescriptors[activeParamDescriptors++] = new ParamDescriptor("heater>modelling>max_core_power", FLOAT,
                                                                           20000,
                                                                           200000,
                                                                           (void *) &settings->heaterSettings.modellerSettings.max_core_power,
                                                                           (void *) &settings->heaterSettings.modellerSettings.max_core_power);
    this->paramDescriptors[activeParamDescriptors++] = new ParamDescriptor("heater>modelling>start_heat_from_cycle",
                                                                           INTEGER,
                                                                           1000,
                                                                           10800,
                                                                           (void *) &settings->heaterSettings.modellerSettings.skip_first_N_cycles,
                                                                           (void *) &settings->heaterSettings.modellerSettings.skip_first_N_cycles);
    this->paramDescriptors[activeParamDescriptors++] = new ParamDescriptor("heater>modelling>core_energy_tr_cf", FLOAT,
                                                                           0.01,
                                                                           0.92,
                                                                           (void *) &settings->heaterSettings.modellerSettings.core_energy_transmitting_coef,
                                                                           (void *) &settings->heaterSettings.modellerSettings.core_energy_transmitting_coef);
    this->paramDescriptors[activeParamDescriptors++] = new ParamDescriptor("heater>modelling>ema_doors_reactions",
                                                                           INTEGER,
                                                                           10,
                                                                           2000,
                                                                           (void *) &settings->heaterSettings.modellerSettings.ema_doors_reactions,
                                                                           (void *) &settings->heaterSettings.modellerSettings.ema_doors_reactions);
    this->paramDescriptors[activeParamDescriptors++] = new ParamDescriptor("heater>modelling>add_core_doors_coef",
                                                                           FLOAT,
                                                                           0.5,
                                                                           3.0,
                                                                           (void *) &settings->heaterSettings.modellerSettings.additional_core_doors_coef,
                                                                           (void *) &settings->heaterSettings.modellerSettings.additional_core_doors_coef);
    this->paramDescriptors[activeParamDescriptors++] = new ParamDescriptor("heater>modelling>home_temperature", FLOAT,
                                                                           -20,
                                                                           45,
                                                                           (void *) &settings->heaterSettings.modellerSettings.radiators_home_temperature,
                                                                           (void *) &settings->heaterSettings.modellerSettings.radiators_home_temperature);
    this->paramDescriptors[activeParamDescriptors++] = new ParamDescriptor("heater>modelling>radiators_base_tempr",
                                                                           FLOAT,
                                                                           60,
                                                                           90,
                                                                           (void *) &settings->heaterSettings.modellerSettings.radiators_base_temperature,
                                                                           (void *) &settings->heaterSettings.modellerSettings.radiators_base_temperature);
    this->paramDescriptors[activeParamDescriptors++] = new ParamDescriptor("heater>modelling>radiators_flow_lpm", FLOAT,
                                                                           0.5,
                                                                           3,
                                                                           (void *) &settings->heaterSettings.modellerSettings.radiators_flow_lpm,
                                                                           (void *) &settings->heaterSettings.modellerSettings.radiators_flow_lpm);
    this->paramDescriptors[activeParamDescriptors++] = new ParamDescriptor("heater>modelling>radiators_eff_coef_pow",
                                                                           FLOAT,
                                                                           0.5,
                                                                           3,
                                                                           (void *) &settings->heaterSettings.modellerSettings.radiators_efficiensy_coef_pow,
                                                                           (void *) &settings->heaterSettings.modellerSettings.radiators_efficiensy_coef_pow);
    this->paramDescriptors[activeParamDescriptors++] = new ParamDescriptor("heater>modelling>pwr_fade_out_steps",
                                                                           INTEGER,
                                                                           100,
                                                                           50000,
                                                                           (void *) &settings->heaterSettings.modellerSettings.power_fade_out_steps,
                                                                           (void *) &settings->heaterSettings.modellerSettings.power_fade_out_steps);
    this->paramDescriptors[activeParamDescriptors++] = new ParamDescriptor("heater>modelling>logging_modeller_info",
                                                                           CHECK_BOX,
                                                                           0,
                                                                           1,
                                                                           (void *) &settings->heaterSettings.modellerSettings.logging_modeller_info,
                                                                           (void *) &settings->heaterSettings.modellerSettings.logging_modeller_info);
    this->paramDescriptors[activeParamDescriptors++] = new ParamDescriptor("heater>modelling>naturaly_cooling_pow",
                                                                           FLOAT,
                                                                           0.01,
                                                                           5,
                                                                           (void *) &settings->heaterSettings.modellerSettings.naturaly_cooling_pow,
                                                                           (void *) &settings->heaterSettings.modellerSettings.naturaly_cooling_pow);
    this->paramDescriptors[activeParamDescriptors++] = new ParamDescriptor("heater>modelling>nat_cooling_coef_div",
                                                                           FLOAT,
                                                                           1000,
                                                                           10000000,
                                                                           (void *) &settings->heaterSettings.modellerSettings.naturaly_cooling_coefficient_as_devider,
                                                                           (void *) &settings->heaterSettings.modellerSettings.naturaly_cooling_coefficient_as_devider);

    for (int stage_index = 0; stage_index < 5; stage_index++) {
        this->paramDescriptors[activeParamDescriptors++] = new ParamDescriptor("heater>modelling>s" + String(stage_index) + "_active", CHECK_BOX,
                                                                               0,
                                                                               1,
                                                                               (void *) &settings->heaterSettings.modellerSettings.stages[stage_index].active,
                                                                               (void *) &settings->heaterSettings.modellerSettings.stages[stage_index].active);
        this->paramDescriptors[activeParamDescriptors++] = new ParamDescriptor("heater>modelling>s" + String(stage_index) + "_start_angle", FLOAT,
                                                                               0.01,
                                                                               90,
                                                                               (void *) &settings->heaterSettings.modellerSettings.stages[stage_index].initial_angle,
                                                                               (void *) &settings->heaterSettings.modellerSettings.stages[stage_index].initial_angle);
        this->paramDescriptors[activeParamDescriptors++] = new ParamDescriptor("heater>modelling>s" + String(stage_index) + "_angle_step", FLOAT,
                                                                               0.01,
                                                                               0.92,
                                                                               (void *) &settings->heaterSettings.modellerSettings.stages[stage_index].core_power_grow_angle_step,
                                                                               (void *) &settings->heaterSettings.modellerSettings.stages[stage_index].core_power_grow_angle_step);
        this->paramDescriptors[activeParamDescriptors++] = new ParamDescriptor("heater>modelling>s" + String(stage_index) + "_max_power_cycles",
                                                                               INTEGER,
                                                                               1000,
                                                                               10800,
                                                                               (void *) &settings->heaterSettings.modellerSettings.stages[stage_index].max_power_cycles,
                                                                               (void *) &settings->heaterSettings.modellerSettings.stages[stage_index].max_power_cycles);
        this->paramDescriptors[activeParamDescriptors++] = new ParamDescriptor(
                "heater>modelling>s" + String(stage_index) + "_radiators_base_power", FLOAT,
                1000,
                180000,
                (void *) &settings->heaterSettings.modellerSettings.stages[stage_index].radiators_base_power,
                (void *) &settings->heaterSettings.modellerSettings.stages[stage_index].radiators_base_power);
        this->paramDescriptors[activeParamDescriptors++] = new ParamDescriptor(
                "heater>modelling>s" + String(stage_index) + "_rad_stop_after_cycle", INTEGER,
                100,
                40000,
                (void *) &settings->heaterSettings.modellerSettings.stages[stage_index].radiator_stop_after_cycle,
                (void *) &settings->heaterSettings.modellerSettings.stages[stage_index].radiator_stop_after_cycle);
        this->paramDescriptors[activeParamDescriptors++] = new ParamDescriptor("heater>modelling>s" + String(stage_index) + "_modeling_length",
                                                                               INTEGER,
                                                                               100,
                                                                               50000,
                                                                               (void *) &settings->heaterSettings.modellerSettings.stages[stage_index].length_of_modeling_cycles,
                                                                               (void *) &settings->heaterSettings.modellerSettings.stages[stage_index].length_of_modeling_cycles);
        this->paramDescriptors[activeParamDescriptors++] = new ParamDescriptor("heater>modelling>s" + String(stage_index) + "_sttmpr_core",
                                                                               FLOAT,
                                                                               1,
                                                                               120,
                                                                               (void *) &settings->heaterSettings.modellerSettings.stages[stage_index].tempr_start_core,
                                                                               (void *) &settings->heaterSettings.modellerSettings.stages[stage_index].tempr_start_core);
        this->paramDescriptors[activeParamDescriptors++] = new ParamDescriptor("heater>modelling>s" + String(stage_index) + "_sttmpr_core_v",
                                                                               FLOAT,
                                                                               1,
                                                                               120,
                                                                               (void *) &settings->heaterSettings.modellerSettings.stages[stage_index].tempr_start_core_volume,
                                                                               (void *) &settings->heaterSettings.modellerSettings.stages[stage_index].tempr_start_core_volume);
        this->paramDescriptors[activeParamDescriptors++] = new ParamDescriptor("heater>modelling>s" + String(stage_index) + "_sttmpr_acc_t",
                                                                               FLOAT,
                                                                               1,
                                                                               120,
                                                                               (void *) &settings->heaterSettings.modellerSettings.stages[stage_index].tempr_start_acc_top,
                                                                               (void *) &settings->heaterSettings.modellerSettings.stages[stage_index].tempr_start_acc_top);
        this->paramDescriptors[activeParamDescriptors++] = new ParamDescriptor("heater>modelling>s" + String(stage_index) + "_sttmpr_acc_h",
                                                                               FLOAT,
                                                                               1,
                                                                               120,
                                                                               (void *) &settings->heaterSettings.modellerSettings.stages[stage_index].tempr_start_acc_higher,
                                                                               (void *) &settings->heaterSettings.modellerSettings.stages[stage_index].tempr_start_acc_higher);
        this->paramDescriptors[activeParamDescriptors++] = new ParamDescriptor("heater>modelling>s" + String(stage_index) + "_sttmpr_acc_l",
                                                                               FLOAT,
                                                                               1,
                                                                               120,
                                                                               (void *) &settings->heaterSettings.modellerSettings.stages[stage_index].tempr_start_acc_lower,
                                                                               (void *) &settings->heaterSettings.modellerSettings.stages[stage_index].tempr_start_acc_lower);
        this->paramDescriptors[activeParamDescriptors++] = new ParamDescriptor("heater>modelling>s" + String(stage_index) + "_sttmpr_acc_b",
                                                                               FLOAT,
                                                                               1,
                                                                               120,
                                                                               (void *) &settings->heaterSettings.modellerSettings.stages[stage_index].tempr_start_acc_bottom,
                                                                               (void *) &settings->heaterSettings.modellerSettings.stages[stage_index].tempr_start_acc_bottom);
    }

}

//--------------------------------------------------------------------
void SettingsNavigator::addParamDescriptor(ParamDescriptor *descriptor) {
    this->paramDescriptors[activeParamDescriptors++] = descriptor;
}

//--------------------------------------------------------------------

String SettingsNavigator::getSettingByName(String origParamName) {
    String paramName = origParamName;

    bool showMinR = paramName.endsWith("#minR");
    bool showMin = paramName.endsWith("#min");
    bool showMaxR = paramName.endsWith("#maxR");
    bool showMax = paramName.endsWith("#max");

    // проверим на наличие #min или #max в конце имени переменной
    if (showMax || showMin) {
        paramName.remove(origParamName.length() - 4);
    } else if (showMaxR || showMinR) {
        paramName.remove(origParamName.length() - 5);
    }

    for (int descriptorIndex = 0; descriptorIndex < activeParamDescriptors; descriptorIndex++) {
        if (paramDescriptors[descriptorIndex]->paramName == paramName) {
            // Нашли наш параметер
            if (paramDescriptors[descriptorIndex]->arraySize == 0 || showMax || showMin || showMaxR || showMinR) {
                if (paramDescriptors[descriptorIndex]->paramType == INTEGER || showMaxR || showMinR) {
                    return showMin || showMax || showMinR || showMaxR
                           ? (showMin || showMinR ? String((int) paramDescriptors[descriptorIndex]->minValue) : String(
                                    (int) paramDescriptors[descriptorIndex]->maxValue))
                           : String(*(int *) paramDescriptors[descriptorIndex]->valueReferenceForRead);
                } else if (paramDescriptors[descriptorIndex]->paramType == BOOLEAN) {
                    return *(bool *) paramDescriptors[descriptorIndex]->valueReferenceForRead ? "true" : "false";
                } else if (paramDescriptors[descriptorIndex]->paramType == CHECK_BOX) {
                    return *(bool *) paramDescriptors[descriptorIndex]->valueReferenceForRead ? "checked" : "";
                } else if (paramDescriptors[descriptorIndex]->paramType == FLOAT) {
                    return showMin || showMax
                           ? (showMin ? String(paramDescriptors[descriptorIndex]->minValue) : String(
                                    paramDescriptors[descriptorIndex]->maxValue))
                           : String(*(float *) paramDescriptors[descriptorIndex]->valueReferenceForRead);
                } else if (paramDescriptors[descriptorIndex]->paramType == STRING) {
                    return showMin || showMax
                           ? (showMin ? String((int) paramDescriptors[descriptorIndex]->minValue) : String(
                                    (int) paramDescriptors[descriptorIndex]->maxValue))
                           : String((char *) paramDescriptors[descriptorIndex]->valueReferenceForRead);
                } else if (paramDescriptors[descriptorIndex]->paramType == UCHAR) {
                    return showMin || showMax
                           ? (showMin ? String((unsigned char) paramDescriptors[descriptorIndex]->minValue) : String(
                                    (unsigned char) paramDescriptors[descriptorIndex]->maxValue))
                           : String(*(unsigned char *) paramDescriptors[descriptorIndex]->valueReferenceForRead);
                } else if (paramDescriptors[descriptorIndex]->paramType == IPV4) {
                    String ipAsString;

                    if (!showMin && !showMax) {
                        unsigned char *ipRaw = (unsigned char *) paramDescriptors[descriptorIndex]->valueReferenceForRead;
                        IPAddress ipV4 = IPAddress(ipRaw[0], ipRaw[1], ipRaw[2], ipRaw[3]);
                        ipAsString = ipV4.toString();
                    }
                    return showMin || showMax
                           ? (showMin ? String((unsigned char) paramDescriptors[descriptorIndex]->minValue) : String(
                                    (unsigned char) paramDescriptors[descriptorIndex]->maxValue))
                           : ipAsString;
                } else if (paramDescriptors[descriptorIndex]->paramType == HEX_BYTES) {
                    // the maximum length of hex string we will made
                    char buffer[257];
                    Converter::bytesToAsciiHex(buffer,
                                               (uint8_t *) paramDescriptors[descriptorIndex]->valueReferenceForRead,
                                               paramDescriptors[descriptorIndex]->byte_data_length);
                    return String(buffer);
                }
            } else {
                //  array
                String paramNameTitle = paramName;
                paramNameTitle.replace('>', '_');
                String result = "{\"" + paramNameTitle + "\":[";

                if (paramDescriptors[descriptorIndex]->paramType == INTEGER) {
                    int *arrayRef = (int *) paramDescriptors[descriptorIndex]->valueReferenceForRead;

                    for (int i = 0; i < paramDescriptors[descriptorIndex]->arraySize; i++) {
                        if (i != 0) {
                            result.concat(",");
                        }
                        result.concat(String(arrayRef[i]));
                    }
                } else if (paramDescriptors[descriptorIndex]->paramType == FLOAT) {
                    float *arrayRef = (float *) paramDescriptors[descriptorIndex]->valueReferenceForRead;

                    for (int i = 0; i < paramDescriptors[descriptorIndex]->arraySize; i++) {
                        if (i != 0) {
                            result.concat(",");
                        }
                        result.concat(String(arrayRef[i]));
                    }
                }
                result.concat("]}");
                return result;
            }
        }
    }

    LOGGER.error("parameter " + paramName + " not found");
    return "bad parameter name";

}
//--------------------------------------------------------------------


void SettingsNavigator::saveNetworkSettingsAndRestart(NetworkSettings *networkSettings) {
    LOGGER.warning("Saving new network settings:\r\n"
                           "        SSID: " + String(networkSettings->ssid) + "\r\n"
                           "    password: " + String(networkSettings->password));

    memcpy(&settings->network, networkSettings, sizeof(NetworkSettings));

    settingsManager->saveSetting(true);
}
//--------------------------------------------------------------------


String SettingsNavigator::saveSettingsByNames(String *params, int paramsCount) {
    String res = "";

    for (int paramIndex = 0; paramIndex < paramsCount; paramIndex++) {
        String param = params[paramIndex];
        param.trim();

        if (!param.isEmpty()) {
// отделим значение от имени параметра
            String invalidParameterMessage = ("Invalid parameter format (\"" + param + "\"");
            int separatorIndex = param.indexOf('=');
            if (separatorIndex == -1) {
                return invalidParameterMessage;
            }

            String paramName = param.substring(0, separatorIndex);
            String value = param.substring(separatorIndex + 1);

            paramName.trim();
            value.trim();

            if (param.isEmpty() || value.isEmpty()) {
                return invalidParameterMessage;
            }

            char tempBuf[64];
            res = saveSettingByName(paramName, value);
        }
        // Если ошибка записи переменной, то далее не продолжаем
        if (res != "") {
            return res;
        }
    }

    settingsManager->saveSetting(false);

    settingsManager->logSettings();

    return res;
}

//--------------------------------------------------------------------
int SettingsNavigator::getParamDescriptorCounter() {
    return activeParamDescriptors;
}

ParamDescriptor *SettingsNavigator::findParamDescriptor(const String &paramName) {
    for (int i = 0; i < activeParamDescriptors; i++) {
        if (paramDescriptors[i] && paramDescriptors[i]->paramName == paramName)
            return paramDescriptors[i];
    }
    return nullptr;
}

void SettingsNavigator::setSensorList(String sensorsList) {
    this->sensorsList = (char *) malloc(sensorsList.length() + 1);
    memcpy(this->sensorsList, sensorsList.c_str(), sensorsList.length());
    this->sensorsList[sensorsList.length()] = 0;
    this->paramDescriptors[activeParamDescriptors++] = new ParamDescriptor("sensors>list", STRING, 0,
                                                                           0,
                                                                           (void *) this->sensorsList,
                                                                           (void *) this->sensorsList);
}
//--------------------------------------------------------------------

String SettingsNavigator::saveSettingByName(String paramName, String value) {
    for (int descriptorIndex = 0; descriptorIndex < activeParamDescriptors; descriptorIndex++) {
        if (paramDescriptors[descriptorIndex]->paramName == paramName) {
            // Нашли наш параметер
            if (paramDescriptors[descriptorIndex]->arraySize == 0) {
                if (paramDescriptors[descriptorIndex]->paramType == INTEGER) {
                    int intValue = value.toInt();
                    if ((intValue < paramDescriptors[descriptorIndex]->minValue)
                        || (intValue > paramDescriptors[descriptorIndex]->maxValue)) {
                        return "Value of \"" + paramName + "\" is not in diapason from "
                               + String(paramDescriptors[descriptorIndex]->minValue) + " to "
                               + String(paramDescriptors[descriptorIndex]->maxValue);
                    } else {
                        *((int *) paramDescriptors[descriptorIndex]->valueReferenceForWrite) = intValue;
                    }
                } else if (paramDescriptors[descriptorIndex]->paramType == UCHAR) {
                    uint8_t ucharValue = value.toInt();
                    if ((ucharValue < paramDescriptors[descriptorIndex]->minValue)
                        || (ucharValue > paramDescriptors[descriptorIndex]->maxValue)) {
                        return "Value of \"" + paramName + "\" is not in diapason from "
                               + String(paramDescriptors[descriptorIndex]->minValue) + " to "
                               + String(paramDescriptors[descriptorIndex]->maxValue);
                    } else {
                        *((uint8_t *) paramDescriptors[descriptorIndex]->valueReferenceForWrite) = ucharValue;
                    }
                } else if (paramDescriptors[descriptorIndex]->paramType == FLOAT) {
                    float floatValue = value.toFloat();
                    if ((floatValue < paramDescriptors[descriptorIndex]->minValue)
                        || (floatValue > paramDescriptors[descriptorIndex]->maxValue)) {
                        return "Value of \"" + paramName + "\" is not in diapason from "
                               + String(paramDescriptors[descriptorIndex]->minValue) + " to "
                               + String(paramDescriptors[descriptorIndex]->maxValue);
                    } else {
                        *((float *) paramDescriptors[descriptorIndex]->valueReferenceForWrite) = floatValue;
                    }
                } else if (paramDescriptors[descriptorIndex]->paramType == STRING) {
                    int valLen = value.length();
                    if ((valLen < paramDescriptors[descriptorIndex]->minValue)
                        || (valLen > paramDescriptors[descriptorIndex]->maxValue)) {
                        return "Length of \"" + paramName + "\" is not in diapason from "
                               + String(paramDescriptors[descriptorIndex]->minValue) + " to "
                               + String(paramDescriptors[descriptorIndex]->maxValue);
                    } else {
                        memset(paramDescriptors[descriptorIndex]->valueReferenceForWrite, 0, valLen + 1);
                        memcpy(paramDescriptors[descriptorIndex]->valueReferenceForWrite, value.c_str(), valLen);
                    }
                } else if (paramDescriptors[descriptorIndex]->paramType == HEX_BYTES) {
                    // the maximum length of hex string we will made
                    if ((value.length() >> 1) != paramDescriptors[descriptorIndex]->byte_data_length)
                        return "Length of \"" + paramName + "\" is " + String(value.length())
                               + " but must be " + String(paramDescriptors[descriptorIndex]->byte_data_length);

                    uint8_t buffer[128];
                    if (!Converter::asciiHexToBytes(&buffer[0], value.c_str(),
                                                    paramDescriptors[descriptorIndex]->byte_data_length))
                        return "Failed to convert " + value + " to bytes";

                    memcpy(paramDescriptors[descriptorIndex]->valueReferenceForWrite, buffer,
                           paramDescriptors[descriptorIndex]->byte_data_length);
                } else if ((paramDescriptors[descriptorIndex]->paramType == BOOLEAN)
                           || (paramDescriptors[descriptorIndex]->paramType == CHECK_BOX))
                    *((bool *) paramDescriptors[descriptorIndex]->valueReferenceForWrite) = value == "true";
                else {
                    return "Unsupported data format " + String(paramDescriptors[descriptorIndex]->paramType);
                }
            } else {
                // массив
                if (paramDescriptors[descriptorIndex]->paramType == FLOAT) {

                    char elementSeparator = ',';

                    int indexOfParamSeparator = value.indexOf(elementSeparator);
                    int startCopyIndex = 0;
                    int matrixIndex = 0;
                    String strValue;
                    if (value.indexOf(elementSeparator) >= 0)
                        do {
                            strValue = indexOfParamSeparator == -1
                                       ? value.substring(startCopyIndex)
                                       : value.substring(startCopyIndex, indexOfParamSeparator);
                            startCopyIndex = indexOfParamSeparator + 1;

                            float *refMatrix = (float *) paramDescriptors[descriptorIndex]->valueReferenceForWrite;
                            float floatValue = strValue.toFloat();

                            if ((floatValue < paramDescriptors[descriptorIndex]->minValue)
                                || (floatValue > paramDescriptors[descriptorIndex]->maxValue)) {
                                return "Value of \"" + paramName + "\" is not in diapason from "
                                       + String(paramDescriptors[descriptorIndex]->minValue) + " to "
                                       + String(paramDescriptors[descriptorIndex]->maxValue);
                            }
                            refMatrix[matrixIndex] = floatValue;
                            matrixIndex++;
                        } while ((indexOfParamSeparator = value.indexOf(elementSeparator, indexOfParamSeparator + 1)) >=
                                 0 &&
                                 matrixIndex < 32);
                    // последний элемент
                    if (startCopyIndex < value.length() && matrixIndex < 32) {
                        strValue = indexOfParamSeparator == -1
                                   ? value.substring(startCopyIndex)
                                   : value.substring(startCopyIndex, indexOfParamSeparator);

                        float *refMatrix = (float *) paramDescriptors[descriptorIndex]->valueReferenceForWrite;
                        float floatValue = strValue.toFloat();

                        if ((floatValue < paramDescriptors[descriptorIndex]->minValue)
                            || (floatValue > paramDescriptors[descriptorIndex]->maxValue)) {
                            return "Value of \"" + paramName + "\" is not in diapason from "
                                   + String(paramDescriptors[descriptorIndex]->minValue) + " to "
                                   + String(paramDescriptors[descriptorIndex]->maxValue);
                        }
                        refMatrix[matrixIndex] = floatValue;
                    }
                }
            }

            return "";
        }
    }
    return "parameter \"" + paramName + "\" not found";
}
//--------------------------------------------------------------------

