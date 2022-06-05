//
// Created by dsporykhin on 25.03.22.
//

#include "HeaterController.h"
#include "Defines.h"

/*
 * === Input variables
 *
 * core_tempr - температура котла
 * input_flow_tempr - температура входного потока
 * output_flow_tempr - температура выходного потока
 *
 * smoke_position - позиция дверцы дымохода
 * oxygen_position - позиция поддува
 * upper_door_position - позиция дверци подсоса воздуха в верхнюю камеру
 *
 * alarm_valve_state - позиция клапана аварийного охлаждения
 *
 * pump1_state - статус активности насоса 1
 * pump2_status - статус активности насоса 2
 *
 * termo_100 - температура перовой, верхней, четверти бака
 * termo_66 - температура второй четверти бака
 * termo_33 - температура третьей четверти бака
 * termo_0 - температура четвертой, нижней, четверти бака
 * hot_water_tempr - температура горячей воды в кранах
 *
 *
 *
 * ==== Output values
 *
 * smoke_position - позиция дверцы дымохода
 * oxygen_position - позиция поддува
 * upper_door_position - позиция дверци подсоса воздуха в верхнюю камеру
 *
 * alarm_valve_state - позиция клапана аварийного охлаждения
 *
 * pump_states
 *      pump1_state - статус активности насоса 1
 *      pump2_status - статус активности насоса 2
 *
 * alarm - индикатор нештатной ситуации
 *
 *
 *
 * === Settings
 *
 * max_core_tempr - максимальная температура котла
 * working_core_tempr - рабочая температура котла
 * max_delta_input_output_tempr - максимальная дельта температур, которую нежелательно превышать
 * target_accumulator_tempr - желательная температура теплоаккумулятора
 *
 *
 * accumulator_capacity_ltr - емкость теплоаккумулятора в литрах
 * pipes_capacity_ltr - емкость труб отопления и батарей
 * boiler_capacity_ltr - емкость бойлера
 * heater_capacity_ltr -  емкость котла
 *
 *
 * === System settings
 *    All
 *      interval_ms                - интервал чтения температур. мс
 *
 *    oxygen_pid_settings
 *      p_value
 *      i_sum
 *      d_value
 *      max_i_value
 *      min_output_value_prcnt   - максимум значения,
 *      max_output_value_prcnt     - диапазон изменения выходного значения
 *
 *    smoke_pid_settings
 *      p_value
 *      i_sum
 *      d_value
 *      max_i_value
 *      min_output_value_prcnt   - максимум значения,
 *      max_output_value_prcnt     - диапазон изменения выходного значения
 *
 *
 * ==== Conversions
 *
 * Расчет теплоемкости системы
 * energy_capacity_g = accumulator_energy_capacity + pipes_energy_capacity + core_energy_capacity
 *                     boiler_energy_capacity
 * accumulator_energy_capacity - доступная энергоемкость теплоаккумулятора
 * pipes_energy_capacity - доступная энергоемкость труб отопления
 * core_energy_capacity -  доступная энергоемкость труб котла
 * boiler_energy_capacity - доступная энергоемкость бойлера
 *
 * accumulator_energy_capacity =  (target_accumulator_tempr - termo_100) * accumulator_capacity_ltr / 4
 *                              + (target_accumulator_tempr - termo_66) * accumulator_capacity_ltr / 4
 *                              + (target_accumulator_tempr - termo_33) * accumulator_capacity_ltr / 4
 *                              + (target_accumulator_tempr - termo_0) * accumulator_capacity_ltr / 4
 *
 * pipes_energy_capacity = (target_accumulator_tempr - termo_100) * pipes_energy_capacity
 *
 * core_energy_capacity = (target_accumulator_tempr - output_flow_tempr) * core_energy_capacity
 *
 * boiler_energy_capacity = (target_accumulator_tempr - hot_water_tempr) * boiler_capacity_ltr
 *
 * available_energy = energy_capacity_g * 4200
 *
 *
 * === Modes
 *
 * start_burning   - первичный розжиг
 *
 * burning_wood    - горение древесины с дымом. Требуется допкислород из верхней дверцы
 *
 * burning_coil   - горение древесного угля. Пиролизных газов почти нет. Допкислород почти не требуется
 *
 * burn_slowing    - Горение замедляется. Тепла выделяется сильно меньше.
 *
 * burn_stopped   - горение почти закончилось. Требуется дозакладка. Возможно нажо уведомить, если есть еще запас теплоемкости
 *
 * reload_fuel    - произведена докладка топлива. Надо перейти к режиму start_burning
 *
 * shutting_down_heater -
 *
 * === Rules
 *
 * (pump1_state && !pump2_state || !pump1_state && pump2_state)
 *
 *
 * === Tasks
 *
 * oxygen_position = f_pid(oxygen_pid_settings, core_tempr)
 *
 * smoke_position
 *      1)  при первичной растопке -  открыта настежь
 *      2) При открывании дверцы котла - открыта настежь
 *      3) При перегреве котла - открыта настежь
 *      4) После первичной растопки переходит в положение 1 (30 от открытого состояния)
 *      5) При обычном горении f_pid_smoke(smoke_pid_settings, pump_states)
 *
*/


HeaterController::HeaterController(GlobalSettings *settings, SensorController *sensorController,
                                   SettingsNavigator *settingsNavigator) {
    this->settings = settings;
    this->heaterSettings = &settings->heaterSettings;
    this->sensorController = sensorController;
    this->settingsNavigator = settingsNavigator;

    smoke_pipe_control = new ServoController(SMOKE_SERVO_PIN, 0,
                                             &heaterSettings->servos_hardware_settings.smoke_servo_settings);
    oxygen_door_control = new ServoController(OXYGEN_SERVO_PIN, 1,
                                              &heaterSettings->servos_hardware_settings.oxygen_servo_settings);
    upper_door_control = new ServoController(UPPER_SERVO_PIN, 2,
                                             &heaterSettings->servos_hardware_settings.upper_door_servo_settings);

    pinMode(EMERGENCY_VALVE_PIN, OUTPUT);
    digitalWrite(EMERGENCY_VALVE_PIN, LOW);

    oxygen_pid = new PID(settingsNavigator, &heaterSettings->oxygen_pid, sensorController, CORE_SENSOR_INDEX,
                         &heaterSettings->temperatureSettings.core_target);

    last_cycle_time = 0;

    pumpsController = new PumpsController(heaterSettings);

    telemetry = new Telemetry(settings);

    previous_core_temperature = 0;
    core_DEMA_temperature = 0;

    resetDEMAtimers();

    two_pump_active_delta_core_input = false;
    two_pump_active_delta_core_output = false;

    flow_ticks = 0;
}

void HeaterController::resetDEMAtimers() {
    DiffEMA_rose_above_zero_at = 0;
    DiffEMA_down_bellow_zero_at = 0;
}

void HeaterController::collectTelemetry() {
    TelemetryDataRecord dataRecord;
    dataRecord.core_temp = (float) sensorController->sensor_data[CORE_SENSOR_INDEX].value;
    dataRecord.output_temp = (float) sensorController->sensor_data[OUTPUT_FLOW_SENSOR_INDEX].value;
    dataRecord.input_temp = (float) sensorController->sensor_data[INPUT_FLOW_SENSOR_INDEX].value;
    dataRecord.accumulator_higher_temp = (float) sensorController->sensor_data[ACC_MID_HI_SENSOR_INDEX].value;
    dataRecord.accumulator_lower_temp = (float) sensorController->sensor_data[ACC_MID_LO_SENSOR_INDEX].value;
    dataRecord.accumulator_bottom_temp = (float) sensorController->sensor_data[ACC_BOTTOM_SENSOR_INDEX].value;
    dataRecord.accumulator_top_temp = (float) sensorController->sensor_data[ACC_TOP_SENSOR_INDEX].value;
    dataRecord.forwar_flow_temp = (float) sensorController->sensor_data[FORWARD_FLOW_SENSOR_INDEX].value;
    dataRecord.backward_flow_temp = (float) sensorController->sensor_data[BACKWARD_FLOW_SENSOR_INDEX].value;

    dataRecord.avarage_backward_flow = 0;
    dataRecord.core_power = (float) current_core_power;
    dataRecord.core_EMA_power = (float) core_EMA_power;
    dataRecord.core_flow = (float) core_flow;

    oxygen_pid->fillPID(dataRecord.pid_p, dataRecord.pid_i, dataRecord.pid_i_sum, dataRecord.pid_d,
                        dataRecord.pid_on_hold, dataRecord.pid_prior_value);
    dataRecord.pid_output = (float) oxygen_pid->getRawValue();

    dataRecord.smoke_door_position = (float) smoke_pipe_control->getAngleGrad();
    dataRecord.upper_door_position = (float) upper_door_control->getAngleGrad();
    dataRecord.oxygen_door_position = (float) oxygen_door_control->getAngleGrad();
    dataRecord.heaterMode = mode;
    dataRecord.core_SMA_diff_tempr = (float) core_DEMA_temperature;
    dataRecord.pump_1_state = pumpsController->pump1->getStateName();
    dataRecord.pump_2_state = pumpsController->pump2 ? pumpsController->pump2->getStateName() : (uint8_t) -1;
    telemetry->addData(&dataRecord);
}

void HeaterController::calcMainCoreCharacteristics() {

    // calc core flow
    if (!heaterSettings->two_pumps_settings.flow_senser_installed) {
        // flow sensor is off. use setting for pumps
        core_flow =
                ((pumpsController->pump1->isOn() ? heaterSettings->two_pumps_settings.first_pump_flow_litters_per_minute
                                                 : 0)
                 + (pumpsController->pump2->isOn()
                    ? heaterSettings->two_pumps_settings.second_pump_flow_litters_per_minute : 0))
                / 60.0 * last_cycle_length / 1000.0;

    } else {
        // sensor present. calc volume using sensor ticks.
        core_flow = flow_ticks;
        flow_ticks = 0;
        core_flow *= heaterSettings->two_pumps_settings.volume_per_one_sensors_tick_litters;
    }

    // calc core DEMA temperature
    if (!previous_core_temperature)
        previous_core_temperature = sensorController->sensor_data[CORE_SENSOR_INDEX].value;
    else {
        double _delta_core_temperature =
                sensorController->sensor_data[CORE_SENSOR_INDEX].value - previous_core_temperature;
        core_DEMA_temperature =
                core_DEMA_temperature * (heaterSettings->temperatureSettings.core_temp_diff_EMA - 1) /
                heaterSettings->temperatureSettings.core_temp_diff_EMA
                + (_delta_core_temperature) /
                  heaterSettings->temperatureSettings.core_temp_diff_EMA;
        previous_core_temperature = sensorController->sensor_data[CORE_SENSOR_INDEX].value;

        // calc core power
        if (!pumpsController->getOnPumpsCount()) {
            // all pumps turned off. Calc power as temperature change for core and core volume
            current_core_power = _delta_core_temperature * heaterSettings->capacities_setting.heater_core_ltr *
                                 WATER_ENERGY_PER_LTR_PER_GRAD;
        } else {
            // calc power using flow and delta between temperatures of input and output flows
            current_core_power = (sensorController->sensor_data[OUTPUT_FLOW_SENSOR_INDEX].value -
                                  sensorController->sensor_data[INPUT_FLOW_SENSOR_INDEX].value)
                                 * WATER_ENERGY_PER_LTR_PER_GRAD * core_flow;
        }

        // calc DEMA POWER
        if (previous_core_power) {
            core_DEMA_power =
                    core_DEMA_power * (heaterSettings->temperatureSettings.core_power_diff_EMA - 1) /
                    heaterSettings->temperatureSettings.core_power_diff_EMA
                    + (current_core_power - previous_core_power) /
                      heaterSettings->temperatureSettings.core_power_diff_EMA;
        }
        previous_core_power = current_core_power;

        // calc EMA POWER
        core_EMA_power =
                core_EMA_power * (heaterSettings->temperatureSettings.core_power_EMA - 1) /
                heaterSettings->temperatureSettings.core_power_EMA
                + current_core_power /
                  heaterSettings->temperatureSettings.core_power_EMA;
    }

    // DEMA down bellow zero. Fix time or do nothing
    if (core_DEMA_temperature < 0) {
        if (!DiffEMA_down_bellow_zero_at)
            DiffEMA_down_bellow_zero_at = millis();
    } else
        // clear time
        DiffEMA_down_bellow_zero_at = 0;

    // DEMA rose  above zero. Fix time or do nothing
    if (core_DEMA_temperature >= 0) {
        if (!DiffEMA_rose_above_zero_at)
            DiffEMA_rose_above_zero_at = millis();
    } else
        // clear time
        DiffEMA_rose_above_zero_at = 0;

}
//-------------------------------------------------------------------

long HeaterController::getTimeDEMABellowZeroSec() {
    return DiffEMA_down_bellow_zero_at
           ? (millis() - DiffEMA_down_bellow_zero_at) / 1000
           : 0;
}
//-------------------------------------------------------------------

long HeaterController::getTimeDEMAAboveZeroSec() {

    return DiffEMA_rose_above_zero_at
           ? (millis() - DiffEMA_rose_above_zero_at) / 1000
           : 0;
}
//-------------------------------------------------------------------

void HeaterController::handleModes() {
    switch (mode & !DOOR_OPENED) {
        case STAND_BY:
            handle_STAND_BY_mode();
            break;
        case WARMING:
            handle_WARMING_mode();
            break;
        case FINAL_COOLING:
            handle_FINAL_COOLING_mode();
            break;
        case PID:
            handle_PID_mode();
            break;
        case OVERHEATED:
            handle_OVERHEATED_mode();
            break;
        case CRITICAL:
            handle_CRITICAL_mode();
            break;
        default:
            LOGGER.warning("");
            enterTo_STAND_BY_mode();
            break;
    }

}
//-------------------------------------------------------------------

void HeaterController::handle() {
    if (sensorController->isHasSensors() && (last_cycle_time == 0 || ((millis() - last_cycle_time) >
                                                                      heaterSettings->scan_interval_ms))) {
        LOGGER.info("work cycle...");

        last_cycle_length = millis() - last_cycle_time;

        last_cycle_time = millis();
        sensorController->fire();

        calcMainCoreCharacteristics();

        handleModes();

        collectTelemetry();
    }
}
//-------------------------------------------------------------------

void HeaterController::enterTo_STAND_BY_mode() {
    LOGGER.info("Entered to STAND_BY mode");
    mode = HeaterMode::STAND_BY;
    resetDEMAtimers();
    // stop all pumps
    pumpsController->setOnPumpsCount(0);

    // close two doors and open smoke
    oxygen_door_control->setAnglePercentage(0);
    upper_door_control->setAnglePercentage(0);
    smoke_pipe_control->setAnglePercentage(66);
}
//-------------------------------------------------------------------

/**
 * handling heater in STAND_BY mode
 */
void HeaterController::handle_STAND_BY_mode() {
    // Go to WARMING mode if temp is reache PID on value
    // Go to WARMING mode if SMA of difference core temp > 0 (e.g. core is warming)
    if (((sensorController->sensor_data[CORE_SENSOR_INDEX].value >=
          heaterSettings->temperatureSettings.start_burn_cycle_on_temperature_up_to)
         && core_DEMA_temperature > 0) || (sensorController->sensor_data[CORE_SENSOR_INDEX].value >=
                                           heaterSettings->oxygen_pid.start_pid_on_temperature_up_to)) {
        // reached temperature that indicates that heater is burning
        enterTo_WARMING_mode();
    } else {
        // process
    }
}
//-------------------------------------------------------------------

void HeaterController::handlePumps() {
    if (heaterSettings->two_pumps_settings.enabled) {
        if (!two_pump_active_delta_core_input && !two_pump_active_delta_core_output) {
            // only one pump now active. Check if you need
            if (two_pump_active_delta_core_output)

                two_pump_active_delta_core_output = (sensorController->sensor_data[CORE_SENSOR_INDEX].value
                                                     - sensorController->sensor_data[OUTPUT_FLOW_SENSOR_INDEX].value) >=
                                                    heaterSettings->two_pumps_settings.start_on_delta_temperature_between_input_and_output;


            two_pump_active_delta_core_input = (sensorController->sensor_data[OUTPUT_FLOW_SENSOR_INDEX].value
                                                - sensorController->sensor_data[INPUT_FLOW_SENSOR_INDEX].value) >=
                                               heaterSettings->two_pumps_settings.start_on_delta_temperature_between_input_and_output;


        }
    } else if (!pumpsController->getOnPumpsCount())
        pumpsController->setOnPumpsCount(1);
}
//-------------------------------------------------------------------

/**
 * initiate WARMING mode
 */
void HeaterController::enterTo_WARMING_mode() {
    LOGGER.info("Entered to WARMING mode");
    mode = HeaterMode::WARMING;
    resetDEMAtimers();
}
//-------------------------------------------------------------------

/**
 * handling WARMING mode
 */
void HeaterController::handle_WARMING_mode() {
    // check if necessary to go to the FINAL COOLING mode
    // check if no warming. (DiffEMA is less than given value)
    if ((core_DEMA_temperature < heaterSettings->warmingSettings.warming_to_cooling_DiffEMA)
        // and DEMA is less than 0 more than given time
        && (getTimeDEMABellowZeroSec() >
            heaterSettings->warmingSettings.go_to_cooling_mode_if_DEMA_less_tan_0_more_than_sec)) {
        // yes. it is.
        enterTo_FINAL_COOLING_mode();
    } else
        // check for PID mode
    if ((core_DEMA_temperature > 0)
        && (sensorController->sensor_data[CORE_SENSOR_INDEX].value >
            heaterSettings->oxygen_pid.start_pid_on_temperature_up_to)) {
        enterTo_PID_mode();
    } else {
        // process

    }
}
//-------------------------------------------------------------------

void HeaterController::enterTo_FINAL_COOLING_mode() {
    LOGGER.info("Entered to FINAL COOLING mode");
    mode = HeaterMode::FINAL_COOLING;
    resetDEMAtimers();

    // stop all pumps
    uint8_t active_pumps = pumpsController->setOnPumpsCount(0);
    LOGGER.info("pumps count = " + String(active_pumps));

    // close upper door
    upper_door_control->setAnglePercentage(0);

    // close oxygen door
    oxygen_door_control->setAnglePercentage(0);

    // open smoke door
    smoke_pipe_control->setAnglePercentage(66);
}
//-------------------------------------------------------------------

void HeaterController::handle_FINAL_COOLING_mode() {
    // check if heater is cooling and
    if ((core_DEMA_temperature < heaterSettings->finalCoolingSettings.cooling_to_standBy_temperature)
        && (getTimeDEMABellowZeroSec() >
            heaterSettings->finalCoolingSettings.go_to_stanby_mode_if_DEMA_less_tan_0_more_than_sec)) {
        enterTo_STAND_BY_mode();
    } else
        //  check if core is warming
        // TODO cooling algorithm
    if (true) {

    } else {
        // process
        // TODO
    }
}
//-------------------------------------------------------------------

void HeaterController::enterTo_PID_mode() {
    LOGGER.info("Entered to PID mode");
    mode = HeaterMode::PID;
    resetDEMAtimers();
}
//-------------------------------------------------------------------

void HeaterController::handle_PID_mode() {
    // check for overheat mode
    if (sensorController->sensor_data[CORE_SENSOR_INDEX].value
        >= heaterSettings->temperatureSettings.core_overheat)
        enterTo_OVERHEATED_mode();
        // check for final cooling mode
    else {
        // TODO
        // process
    }
}
//-------------------------------------------------------------------

void HeaterController::enterTo_OVERHEATED_mode() {
    LOGGER.info("Entered to OVERHEATED mode");
    mode = HeaterMode::OVERHEATED;
    // open smoke door
    smoke_pipe_control->setAnglePercentage(100);

    // open upper door
    upper_door_control->setAnglePercentage(100);

    // oxygen door
    oxygen_door_control->setAnglePercentage(0);

    // on two pumps
    pumpsController->setOnPumpsCount(2);

    resetDEMAtimers();
}
//-------------------------------------------------------------------

void HeaterController::handle_OVERHEATED_mode() {
    // check for critical
    if (sensorController->sensor_data[CORE_SENSOR_INDEX].value
        >= heaterSettings->temperatureSettings.core_critical)
        enterTo_CRITICAL_mode();
    else
        // handle for PID mode
    if (sensorController->sensor_data[CORE_SENSOR_INDEX].value
        < heaterSettings->temperatureSettings.core_overheat)
        enterTo_PID_mode();
}
//-------------------------------------------------------------------

void HeaterController::enterTo_CRITICAL_mode() {
    LOGGER.info("Entered to CRITICAL mode");
    mode = HeaterMode::CRITICAL;
    // open smoke door
    smoke_pipe_control->setAnglePercentage(100);

    // open upper door
    upper_door_control->setAnglePercentage(100);

    // oxygen door
    oxygen_door_control->setAnglePercentage(0);

    // on two pumps
    pumpsController->setOnPumpsCount(2);

    // open emergency valve
    digitalWrite(EMERGENCY_VALVE_PIN, HIGH);

    resetDEMAtimers();

}
//-------------------------------------------------------------------

void HeaterController::handle_CRITICAL_mode() {
    // check for overheated mode
    if (sensorController->sensor_data[CORE_SENSOR_INDEX].value
        < heaterSettings->temperatureSettings.core_critical) {
        digitalWrite(EMERGENCY_VALVE_PIN, LOW);
        enterTo_OVERHEATED_mode();
    }
}
//-------------------------------------------------------------------

void HeaterController::handle_DOOR_OPENED_mode() {
    // TODO
}
//-------------------------------------------------------------------
//-------------------------------------------------------------------

/**
 * rule by pumps
 */
void HeaterController::handleTwoPumps() {
    switch (mode) {
        case STAND_BY :
            if (pumpsController->getOnPumpsCount())
                pumpsController->setOnPumpsCount(0);
            break;
        case WARMING:
        case PID:
            handlePumps();
        case FINAL_COOLING:
//            if ()
                break;
        case CRITICAL:
        case OVERHEATED:
        default:
            pumpsController->setOnPumpsCount(2);
            break;

    }
}
//-------------------------------------------------------------------

