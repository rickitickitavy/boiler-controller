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

    if (EMERGENCY_VALVE_PIN) {
        pinMode(EMERGENCY_VALVE_PIN, OUTPUT);
        digitalWrite(EMERGENCY_VALVE_PIN, LOW);
    }

    doorsController = new DoorsController(&heaterSettings->servos_hardware_settings);

    pidRegulator = new PidRegulator(heaterSettings, sensorController, &mainCoreParams);

    last_cycle_time = 0;

    pumpsController = new PumpsController(heaterSettings);

    telemetry = new Telemetry(settings);

    previous_core_temperature = 0;
    mainCoreParams.core_DEMA_temperature = 0;

    resetDEMAtimers();

    two_pump_active_delta_core_input = false;
    two_pump_active_delta_core_output = false;

    flow_ticks = 0;
}

void HeaterController::resetDEMAtimers() {
    mainCoreParams.DiffEMA_rose_above_zero_at = 0;
    mainCoreParams.DiffEMA_down_bellow_zero_at = 0;
}

void HeaterController::collectTelemetry(long last_cycle_length) {
    TelemetryDataRecord dataRecord;
    dataRecord.interval_ms = last_cycle_length;
    dataRecord.date_time_ms = millis();
    dataRecord.core_temp = (float) sensorController->sensor_data[T_SENS_INDEX_CORE].value;
    dataRecord.output_temp = (float) sensorController->sensor_data[T_SENS_INDEX_OUTPUT_FLOW].value;
    dataRecord.input_temp = (float) sensorController->sensor_data[T_SENS_INDEX_INPUT_FLOW].value;
    dataRecord.accumulator_higher_temp = (float) sensorController->sensor_data[T_SENS_INDEX_ACC_MID_HI].value;
    dataRecord.accumulator_lower_temp = (float) sensorController->sensor_data[T_SENS_INDEX_ACC_MID_LO].value;
    dataRecord.accumulator_bottom_temp = (float) sensorController->sensor_data[T_SENS_INDEX_ACC_BOTTOM].value;
    dataRecord.accumulator_top_temp = (float) sensorController->sensor_data[T_SENS_INDEX_ACC_TOP].value;
    dataRecord.forwar_flow_temp = (float) sensorController->sensor_data[T_SENS_INDEX_FORWARD_FLOW].value;
    dataRecord.backward_flow_temp = (float) sensorController->sensor_data[T_SENS_INDEX_BACKWARD_FLOW].value;

    dataRecord.core_temp_sma = (float) sensorController->getSmaValue(T_SENS_INDEX_CORE);
    dataRecord.output_temp_sma = (float) sensorController->getSmaValue(T_SENS_INDEX_OUTPUT_FLOW);
    dataRecord.input_temp_sma = (float) sensorController->getSmaValue(T_SENS_INDEX_INPUT_FLOW);
    dataRecord.accumulator_higher_temp_sma = (float) sensorController->getSmaValue(T_SENS_INDEX_ACC_MID_HI);
    dataRecord.accumulator_lower_temp_sma = (float) sensorController->getSmaValue(T_SENS_INDEX_ACC_MID_LO);
    dataRecord.accumulator_bottom_temp_sma = (float) sensorController->getSmaValue(T_SENS_INDEX_ACC_BOTTOM);
    dataRecord.accumulator_top_temp_sma = (float) sensorController->getSmaValue(T_SENS_INDEX_ACC_TOP);
    dataRecord.forwar_flow_temp_sma = (float) sensorController->getSmaValue(T_SENS_INDEX_FORWARD_FLOW);
    dataRecord.backward_flow_temp_sma = (float) sensorController->getSmaValue(T_SENS_INDEX_BACKWARD_FLOW);

    dataRecord.avarage_backward_flow = 0;
    dataRecord.core_power = (float) mainCoreParams.current_core_power;
    dataRecord.core_EMA_power = (float) mainCoreParams.core_EMA_power;
    dataRecord.core_flow = (float) core_flow;

    pidRegulator->fillPID(dataRecord.pid_p, dataRecord.pid_i, dataRecord.pid_i_sum, dataRecord.pid_d,
                          dataRecord.pid_on_hold, dataRecord.pid_prior_value);
    dataRecord.pid_output = (float) pidRegulator->getRawValue();

    dataRecord.smoke_door_position = (float) doorsController->getSmokePipeValue();
    dataRecord.upper_door_position = (float) doorsController->getUpperDoorValue();
    dataRecord.oxygen_door_position = (float) doorsController->getOxygenDoorValue();
    dataRecord.heaterMode = mode;
    dataRecord.core_SMA_diff_tempr = (float) mainCoreParams.core_DEMA_temperature;
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
    if (!previous_core_temperature) {
        previous_core_temperature = sensorController->getSmaValue(T_SENS_INDEX_CORE);
        mainCoreParams.core_DEMA_temperature = 0;
        mainCoreParams.current_core_power = 0;
        mainCoreParams.core_EMA_power = 0;
        mainCoreParams.core_DEMA_power = 0;
    } else {
        double _delta_core_temperature =
                sensorController->getSmaValue(T_SENS_INDEX_CORE) - previous_core_temperature;
//        Serial.println(" --- debug in: _delta_core_temperature = " + String( _delta_core_temperature));

        mainCoreParams.core_DEMA_temperature =
                mainCoreParams.core_DEMA_temperature * (heaterSettings->temperatureSettings.core_temp_diff_EMA - 1) /
                heaterSettings->temperatureSettings.core_temp_diff_EMA
                + (_delta_core_temperature) /
                  heaterSettings->temperatureSettings.core_temp_diff_EMA;
        previous_core_temperature = sensorController->getSmaValue(T_SENS_INDEX_CORE);

        // calc core power
        if (!pumpsController->getOnPumpsCount()) {
//            Serial.println(" core_pwr branch 1");
            // all pumps turned off. Calc power as temperature change for core and core volume
            mainCoreParams.current_core_power =
                    _delta_core_temperature * heaterSettings->capacities_setting.heater_core_ltr *
                    WATER_ENERGY_PER_LTR_PER_GRAD / (double)last_cycle_length * 1000;
        } else {
//            Serial.println(" core_pwr branch 2");
            // calc power using flow and delta between temperatures of input and output flows
            mainCoreParams.current_core_power = (sensorController->getSmaValue(T_SENS_INDEX_OUTPUT_FLOW) -
                                                 sensorController->getSmaValue(T_SENS_INDEX_INPUT_FLOW)
                                                * WATER_ENERGY_PER_LTR_PER_GRAD * core_flow);
        }

        // calc DEMA POWER
        if (previous_core_power) {
            mainCoreParams.core_DEMA_power =
                    mainCoreParams.core_DEMA_power * (heaterSettings->temperatureSettings.core_power_diff_EMA - 1) /
                    heaterSettings->temperatureSettings.core_power_diff_EMA
                    + (mainCoreParams.current_core_power - previous_core_power) /
                      heaterSettings->temperatureSettings.core_power_diff_EMA;
        }
        previous_core_power = mainCoreParams.current_core_power;

        // calc EMA POWER
        mainCoreParams.core_EMA_power =
                mainCoreParams.core_EMA_power * (heaterSettings->temperatureSettings.core_power_EMA - 1) /
                heaterSettings->temperatureSettings.core_power_EMA
                + (isnan(mainCoreParams.current_core_power) ? 0 : mainCoreParams.current_core_power) /
                  heaterSettings->temperatureSettings.core_power_EMA;
    }

    Serial.println(" -- debug: mainCoreParams.core_EMA_power = " + (isnan(mainCoreParams.core_EMA_power) ? " nan" : String(mainCoreParams.core_EMA_power)));

    // DEMA down bellow zero. Fix time or do nothing
    if (mainCoreParams.core_DEMA_temperature < 0) {
        if (!mainCoreParams.DiffEMA_down_bellow_zero_at)
            mainCoreParams.DiffEMA_down_bellow_zero_at = millis();
    } else
        // clear time
        mainCoreParams.DiffEMA_down_bellow_zero_at = 0;

    // DEMA rose  above zero. Fix time or do nothing
    if (mainCoreParams.core_DEMA_temperature >= 0) {
        if (!mainCoreParams.DiffEMA_rose_above_zero_at)
            mainCoreParams.DiffEMA_rose_above_zero_at = millis();
    } else
        // clear time
        mainCoreParams.DiffEMA_rose_above_zero_at = 0;
}
//-------------------------------------------------------------------

long HeaterController::getTimeDEMABellowZeroSec() {
    return mainCoreParams.DiffEMA_down_bellow_zero_at
           ? (millis() - mainCoreParams.DiffEMA_down_bellow_zero_at) / 1000
           : 0;
}
//-------------------------------------------------------------------

long HeaterController::getTimeDEMAAboveZeroSec() {

    return mainCoreParams.DiffEMA_rose_above_zero_at
           ? (millis() - mainCoreParams.DiffEMA_rose_above_zero_at) / 1000
           : 0;
}
//-------------------------------------------------------------------

void HeaterController::handleModes() {
    // hardcoded failsafe
    if (sensorController->getSmaValue(T_SENS_INDEX_CORE) >= 98)
        switchTo_OVERHEATED_mode();
    else
        switch (mode & (DOOR_OPENED ^ 0xff)) {
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
                switchTo_STAND_BY_mode();
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

        LOGGER.info(" last_cycle_length = " + String(last_cycle_length));

        calcMainCoreCharacteristics();

        handleModes();

        handlePumps();

        collectTelemetry(last_cycle_length);
    }
}
//-------------------------------------------------------------------

void HeaterController::switchTo_STAND_BY_mode() {
    LOGGER.info("Entered to STAND_BY mode");
    mode = HeaterMode::STAND_BY;
    resetDEMAtimers();

    // stop all pumps
    pumpsController->setOnPumpsCount(0);
    standby_cooling_active = false;

    // close two doors and open smoke
    doorsController->setOxygenDoorValue(0);
    doorsController->setUpperDoorValue(0);
    doorsController->setSmokePipeValue(66);
}
//-------------------------------------------------------------------

/**
 * handling heater in STAND_BY mode
 */
void HeaterController::handle_STAND_BY_mode() {
    // if tempr > overheat - go to overheatmode
    if (sensorController->getSmaValue(T_SENS_INDEX_CORE) >=
        heaterSettings->temperatureSettings.core_overheat)
        switchTo_OVERHEATED_mode();
    else
        // if power > burn power -  start warming mode
    if (mainCoreParams.core_EMA_power > heaterSettings->stadbyCoolingSettings.start_warming_cycle_on_power)
        switchTo_WARMING_mode();
    else {
    }
}
//-------------------------------------------------------------------

void HeaterController::handlePumps() {
    if ((mode == STAND_BY) || (mode == FINAL_COOLING)) {
        // cooling only
        // if tempr > pid_on tempr then core need to be cooled
        bool _cooling_expected = sensorController->getSmaValue(T_SENS_INDEX_CORE) >=
                                 heaterSettings->coolingByPumpsSettings.start_pumps_temperature;

        // if cooling expected then check for input flow temperature is less than core for XX or more degrees
        if (_cooling_expected)
            _cooling_expected = (sensorController->getSmaValue(T_SENS_INDEX_CORE) -
                                 sensorController->getSmaValue(T_SENS_INDEX_INPUT_FLOW)) >=
                                heaterSettings->coolingByPumpsSettings.min_delta_btw_core_and_input_to_start_pumps;

        // it is already on check that temp
        if (_cooling_expected)
            _cooling_expected = !standby_cooling_active ||
                                (standby_cooling_active
                                 && ((heaterSettings->coolingByPumpsSettings.start_pumps_temperature
                                      - sensorController->getSmaValue(T_SENS_INDEX_CORE)) <
                                     heaterSettings->coolingByPumpsSettings.delta_btw_start_and_core_to_stop_pumps));

        if (_cooling_expected) {
            if (!standby_cooling_active) {
                standby_cooling_active = true;
                pumpsController->setOnPumpsCount(2);
            }
        } else if (standby_cooling_active) {
            standby_cooling_active = false;
            pumpsController->setOnPumpsCount(0);
        }
    } else if (heaterSettings->two_pumps_settings.enabled) {
        if (!two_pump_active_delta_core_input && !two_pump_active_delta_core_output) {
            // only one pump now active. Check if you need
            if (two_pump_active_delta_core_output)

                two_pump_active_delta_core_output = (sensorController->getSmaValue(T_SENS_INDEX_CORE)
                                                     - sensorController->getSmaValue(T_SENS_INDEX_OUTPUT_FLOW)) >=
                                                    heaterSettings->two_pumps_settings.start_on_delta_temperature_between_input_and_output;


            two_pump_active_delta_core_input = (sensorController->getSmaValue(T_SENS_INDEX_OUTPUT_FLOW)
                                                - sensorController->getSmaValue(T_SENS_INDEX_INPUT_FLOW)) >=
                                               heaterSettings->two_pumps_settings.start_on_delta_temperature_between_input_and_output;
        }
    } else if (!pumpsController->getOnPumpsCount())
        pumpsController->setOnPumpsCount(1);
}
//-------------------------------------------------------------------

/**
 * initiate WARMING mode
 */
void HeaterController::switchTo_WARMING_mode() {
    LOGGER.info("Entered to WARMING mode");
    mode = HeaterMode::WARMING;
    entered_to_warming_mode_at = millis();
    pumpsController->setOnPumpsCount(2);
    resetDEMAtimers();
}
//-------------------------------------------------------------------

/**
 * handling WARMING mode
 */
void HeaterController::handle_WARMING_mode() {
    // check if power reached target power to switch to PID mode
    if ((mainCoreParams.core_EMA_power >= heaterSettings->warmingSettings.target_power_to_switch_to_the_PID_mode)
        || (sensorController->getSmaValue(T_SENS_INDEX_CORE) >
            heaterSettings->warmingSettings.start_pid_temperature))
        switchTo_PID_mode();
    else
        // check if time to reach target power is up
    if ((millis() - entered_to_warming_mode_at) / 1000 > heaterSettings->warmingSettings.time_to_reach_target_power_sec)
        switchTo_FINAL_COOLING_mode();
    else {
        // process. if needed
    }
}
//-------------------------------------------------------------------

void HeaterController::switchTo_FINAL_COOLING_mode() {
    LOGGER.info("Entered to FINAL COOLING mode");
    mode = HeaterMode::FINAL_COOLING;
    resetDEMAtimers();

    final_cooling_power_low_at = 0;

    // stop all pumps
    uint8_t active_pumps = pumpsController->setOnPumpsCount(0);
    LOGGER.info("pumps count = " + String(active_pumps));

    // close upper door
    doorsController->setUpperDoorValue(0);

    // close oxygen door
    doorsController->setOxygenDoorValue(0);

    // open smoke door
    doorsController->setSmokePipeValue(66);
}
//-------------------------------------------------------------------

void HeaterController::handle_FINAL_COOLING_mode() {
    // check if power EMA down bellow power switching from the standby mode to the warming mode
    if (mainCoreParams.core_EMA_power < heaterSettings->stadbyCoolingSettings.start_warming_cycle_on_power) {
        if (!final_cooling_power_low_at)
            final_cooling_power_low_at = millis();
        else if (((millis() - final_cooling_power_low_at) / 1000
                  < heaterSettings->finalCoolingSettings.delay_to_swirtch_to_standby_mode_sec)
                 && (sensorController->getSmaValue(T_SENS_INDEX_CORE)
                     <= heaterSettings->finalCoolingSettings.max_temperature_to_switch_to_standBy))
            switchTo_STAND_BY_mode();
    } else {
        final_cooling_power_low_at = 0;
        // check for power up to power_to_switch_to_warming_mode. If it is true - switch to the warming mode
        if (mainCoreParams.core_EMA_power >= heaterSettings->finalCoolingSettings.power_to_switch_to_warming_mode)
            switchTo_WARMING_mode();
    }
    // process
    // TODO
}
//-------------------------------------------------------------------

void HeaterController::switchTo_PID_mode() {
    LOGGER.info("Entered to PID mode");
    mode = HeaterMode::PID;
    resetDEMAtimers();
}
//-------------------------------------------------------------------

void HeaterController::handle_PID_mode() {
    // check for overheat mode
    if (sensorController->getSmaValue(T_SENS_INDEX_CORE)
        >= heaterSettings->temperatureSettings.core_overheat)
        switchTo_OVERHEATED_mode();
        // check for back to warming mode
    else if (mainCoreParams.core_EMA_power < heaterSettings->warmingSettings.target_power_to_switch_to_the_PID_mode)
        switchTo_WARMING_mode();
    else {
        pidRegulator->handle();
        doorsController->setOxygenDoorValue(pidRegulator->getValuePrcnt());
        // TODO
        // process
    }
}
//-------------------------------------------------------------------

void HeaterController::switchTo_OVERHEATED_mode() {
    LOGGER.info("Entered to OVERHEATED mode");
    mode = HeaterMode::OVERHEATED;
    // open smoke door
    doorsController->setSmokePipeValue(100);

    // open upper door
    doorsController->setUpperDoorValue(100);

    // oxygen door
    doorsController->setOxygenDoorValue(0);

    // on two pumps
    pumpsController->setOnPumpsCount(2);

    resetDEMAtimers();
}
//-------------------------------------------------------------------

void HeaterController::handle_OVERHEATED_mode() {
    // check for critical
    if (sensorController->getSmaValue(T_SENS_INDEX_CORE)
        >= heaterSettings->temperatureSettings.core_critical)
        switchTo_CRITICAL_mode();
    else
        // handle for PID mode
    if (sensorController->getSmaValue(T_SENS_INDEX_CORE)
        < heaterSettings->temperatureSettings.core_overheat)
        switchTo_PID_mode();
}
//-------------------------------------------------------------------

void HeaterController::switchTo_CRITICAL_mode() {
    LOGGER.info("Entered to CRITICAL mode");
    mode = HeaterMode::CRITICAL;
    // open smoke door
    doorsController->setSmokePipeValue(100);

    // open upper door
    doorsController->setUpperDoorValue(100);

    // oxygen door
    doorsController->setOxygenDoorValue(0);

    // on two pumps
    pumpsController->setOnPumpsCount(2);

    // open emergency valve
    digitalWrite(EMERGENCY_VALVE_PIN, HIGH);

    resetDEMAtimers();

}
//-------------------------------------------------------------------

void HeaterController::handle_CRITICAL_mode() {
    // check for overheated mode
    if (sensorController->getSmaValue(T_SENS_INDEX_CORE)
        < heaterSettings->temperatureSettings.core_critical) {
        digitalWrite(EMERGENCY_VALVE_PIN, LOW);
        switchTo_OVERHEATED_mode();
    }
}
//-------------------------------------------------------------------

void HeaterController::handle_DOOR_OPENED_mode() {
    // TODO
}
//-------------------------------------------------------------------
//-------------------------------------------------------------------
//-------------------------------------------------------------------

