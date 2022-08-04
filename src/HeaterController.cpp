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

HeaterController * HeaterController::instance = nullptr;

HeaterController::HeaterController(GlobalSettings *settings, SensorController *sensorController,
                                   SettingsNavigator *settingsNavigator) {
    this->settings = settings;
    this->heaterSettings = &settings->heaterSettings;
    this->sensorController = sensorController;
    this->settingsNavigator = settingsNavigator;
    instance = this;
    modelling_is_active = false;
    flowSensor = new FlowSensor(FLOW_SENSOR_PIN);

    if (EMERGENCY_VALVE_PIN) {
        pinMode(EMERGENCY_VALVE_PIN, OUTPUT);
        digitalWrite(EMERGENCY_VALVE_PIN, LOW);
    }

    cycle_index = 0;

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

    coreModel = new CoreModel(&settings->heaterSettings, pumpsController, doorsController, telemetry);

    time_to_close_oxygen_door_in_stanby_mode = 0;

    cycle_index = 10000;
    estimated_modelling_cycle_counter = 0;

    settingsNavigator->addParamDescriptor(new ParamDescriptor("heater>modelling>est_modelling_cycles", INTEGER,
                                                              100,
                                                              50000,
                                                              (void *) &estimated_modelling_cycle_counter,
                                                              (void *) nullptr));

    settingsNavigator->addParamDescriptor(new ParamDescriptor("heater>modelling>current_stage", INTEGER,
                                                              100,
                                                              50000,
                                                              (void *) &coreModel->stage_index,
                                                              (void *) nullptr));

    pinMode(MAIN_DOOR_SENSOR_PIN, INPUT_PULLDOWN);
    attachInterrupt(MAIN_DOOR_SENSOR_PIN, MAIN_DOOR_ISR, CHANGE);
    MAIN_DOOR_ISR();
    previous_main_door_opened = false;
}

void HeaterController::resetDEMAtimers() {
    mainCoreParams.DiffEMA_rose_above_zero_at = 0;
    mainCoreParams.DiffEMA_down_bellow_zero_at = 0;
}

void IRAM_ATTR HeaterController::MAIN_DOOR_ISR() {
    bool door_mode = digitalRead(MAIN_DOOR_SENSOR_PIN);
    instance->main_door_opened = !door_mode;
}

void HeaterController::collectTelemetry(long last_cycle_length) {
    telemetryDataRecord.interval_ms = last_cycle_length;
    telemetryDataRecord.date_time_ms = millis();
    telemetryDataRecord.core_temp = (float) sensorController->sensor_data[T_SENS_INDEX_CORE].value;
    telemetryDataRecord.output_temp = (float) sensorController->sensor_data[T_SENS_INDEX_OUTPUT_FLOW].value;
    telemetryDataRecord.input_temp = (float) sensorController->sensor_data[T_SENS_INDEX_INPUT_FLOW].value;
    telemetryDataRecord.accumulator_higher_temp = (float) sensorController->sensor_data[T_SENS_INDEX_ACC_MID_HI].value;
    telemetryDataRecord.accumulator_lower_temp = (float) sensorController->sensor_data[T_SENS_INDEX_ACC_MID_LO].value;
    telemetryDataRecord.accumulator_bottom_temp = (float) sensorController->sensor_data[T_SENS_INDEX_ACC_BOTTOM].value;
    telemetryDataRecord.accumulator_top_temp = (float) sensorController->sensor_data[T_SENS_INDEX_ACC_TOP].value;
    telemetryDataRecord.forwar_flow_temp = (float) sensorController->sensor_data[T_SENS_INDEX_FORWARD_FLOW].value;
    telemetryDataRecord.backward_flow_temp = (float) sensorController->sensor_data[T_SENS_INDEX_BACKWARD_FLOW].value;

    telemetryDataRecord.internal_temp = (float) sensorController->getNotNANValue(T_SENS_INDEX_INTERNAL);
    telemetryDataRecord.core_temp_sma = (float) sensorController->getNotNANSmaValue(T_SENS_INDEX_CORE);
    telemetryDataRecord.output_temp_sma = (float) sensorController->getNotNANSmaValue(T_SENS_INDEX_OUTPUT_FLOW);
    telemetryDataRecord.input_temp_sma = (float) sensorController->getNotNANSmaValue(T_SENS_INDEX_INPUT_FLOW);
    telemetryDataRecord.accumulator_higher_temp_sma = (float) sensorController->getNotNANSmaValue(T_SENS_INDEX_ACC_MID_HI);
    telemetryDataRecord.accumulator_lower_temp_sma = (float) sensorController->getNotNANSmaValue(T_SENS_INDEX_ACC_MID_LO);
    telemetryDataRecord.accumulator_bottom_temp_sma = (float) sensorController->getNotNANSmaValue(T_SENS_INDEX_ACC_BOTTOM);
    telemetryDataRecord.accumulator_top_temp_sma = (float) sensorController->getNotNANSmaValue(T_SENS_INDEX_ACC_TOP);
    telemetryDataRecord.forwar_flow_temp_sma = (float) sensorController->getNotNANSmaValue(T_SENS_INDEX_FORWARD_FLOW);
    telemetryDataRecord.backward_flow_temp_sma = (float) sensorController->getNotNANSmaValue(T_SENS_INDEX_BACKWARD_FLOW);

    telemetryDataRecord.avarage_backward_flow = 0;
    telemetryDataRecord.core_power = (float) mainCoreParams.current_core_power;
    telemetryDataRecord.core_EMA_power = (float) mainCoreParams.core_EMA_power;
    telemetryDataRecord.core_flow = (float) mainCoreParams.core_flow * 1000 / last_cycle_length * 60.0;

    pidRegulator->fillPID(telemetryDataRecord.pid_p, telemetryDataRecord.pid_i, telemetryDataRecord.pid_d,
                          telemetryDataRecord.pid_raw_output, telemetryDataRecord.pid_output);

    telemetryDataRecord.smoke_door_position = (float) doorsController->getSmokePipeAngle();
    telemetryDataRecord.upper_door_position = (float) doorsController->getUpperDoorAngle();
    telemetryDataRecord.oxygen_door_position = (float) doorsController->getOxygenDoorAngle();
    telemetryDataRecord.heaterMode = mode;
    telemetryDataRecord.core_SMA_diff_tempr = (float) mainCoreParams.core_DEMA_temperature;
    telemetryDataRecord.pump_1_state = pumpsController->pump1->getStateName();
    telemetryDataRecord.pump_2_state = pumpsController->pump2 ? pumpsController->pump2->getStateName() : (uint8_t) -1;
    telemetry->addData(&telemetryDataRecord);
    telemetryDataRecord.main_door_opened = main_door_opened;
}

void HeaterController::calcMainCoreCharacteristics(long last_cycle_length) {
    // calc core flow
    flow_ticks = flowSensor->readAndReset();
    if (!heaterSettings->two_pumps_settings.flow_senser_installed) {
        // flow sensor is off. use setting for pumps
        mainCoreParams.core_flow =
                ((pumpsController->pump1->isOn() ? heaterSettings->two_pumps_settings.first_pump_flow_litters_per_minute
                                                 : 0)
                 + (pumpsController->pump2->isOn()
                    ? heaterSettings->two_pumps_settings.second_pump_flow_litters_per_minute : 0))
                / 60.0 * last_cycle_length / 1000.0;

    } else {
        // sensor present. calc volume using sensor ticks.
        mainCoreParams.core_flow = flow_ticks;
        flow_ticks = 0;
        mainCoreParams.core_flow /= heaterSettings->two_pumps_settings.flow_sensor_ticks_per_litters;
    }

    mainCoreParams.core_temperature = sensorController->getSmaValue(T_SENS_INDEX_CORE);

    // calc core DEMA temperature
    if (!previous_core_temperature) {
        previous_core_temperature = mainCoreParams.core_temperature;
        mainCoreParams.core_DEMA_temperature = 0;
        mainCoreParams.current_core_power = 0;
        mainCoreParams.core_EMA_power = 0;
        mainCoreParams.core_DEMA_power = 0;
    } else {

        double _delta_core_temperature = mainCoreParams.core_temperature - previous_core_temperature;
//        Serial.println(" --- debug in: _delta_core_temperature = " + String( _delta_core_temperature));

        mainCoreParams.core_DEMA_temperature =
                mainCoreParams.core_DEMA_temperature * (heaterSettings->temperatureSettings.core_temp_diff_EMA - 1) /
                heaterSettings->temperatureSettings.core_temp_diff_EMA
                + (_delta_core_temperature) /
                  heaterSettings->temperatureSettings.core_temp_diff_EMA;
        previous_core_temperature = mainCoreParams.core_temperature;

        // calc core power
        if (!pumpsController->getOnPumpsCount()) {
            // all pumps turned off. Calc power as temperature change for core and core volume
            mainCoreParams.current_core_power =
                    _delta_core_temperature * heaterSettings->capacities_setting.heater_core_ltr *
                    WATER_ENERGY_PER_LTR_PER_GRAD / (double) last_cycle_length * 1000;
        } else {
            // calc power using flow and delta between temperatures of input and output flows
            mainCoreParams.current_core_power = (sensorController->getSmaValue(T_SENS_INDEX_OUTPUT_FLOW) -
                                                 sensorController->getSmaValue(T_SENS_INDEX_INPUT_FLOW))
                                                * WATER_ENERGY_PER_LTR_PER_GRAD * mainCoreParams.core_flow
                                                / (double) last_cycle_length * 1000;
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
    // in all cases pid must calculate
    if (!isnan(sensorController->getSmaValue(T_SENS_INDEX_CORE)))
        pidRegulator->handle();

    // hardcoded failsafe
    if (sensorController->getSmaValue(T_SENS_INDEX_CORE) >= 100)
        switchTo_CRITICAL_mode();
    else
        switch (mode) {
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
//                pidRegulator->handle();
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

void HeaterController::openOxygenDoorForTime(long time_sec) {
    if (mode == STAND_BY) {
        LOGGER.info("Oxygen door for " + String(time_sec) + " seconds opened.");
        doorsController->setOxygenDoorValue(100);
        time_to_close_oxygen_door_in_stanby_mode = millis() + time_sec * 1000;
    }
}
//-------------------------------------------------------------------

void HeaterController::closeOxygenDoor() {
    if (mode == STAND_BY) {
        LOGGER.info("Oxygen door closing...");
        if ((time_to_close_oxygen_door_in_stanby_mode != 0) && (millis() < time_to_close_oxygen_door_in_stanby_mode)) {
            doorsController->setOxygenDoorValue(0);
            time_to_close_oxygen_door_in_stanby_mode = 0;
            LOGGER.info("Oxygen door closed.");
        } else
            LOGGER.info("Oxygen door already closed.");

    }
}
//-------------------------------------------------------------------

bool HeaterController::handle() {
    bool proceeded = false;
    if ((sensorController->isHasSensors() && (last_cycle_time == 0 || ((millis() - last_cycle_time) >
                                                                       heaterSettings->scan_interval_ms)))
        || modelling_is_active) {
        proceeded = true;
        cycle_index++;

        if (modelling_is_active) {
            if (estimated_modelling_cycle_counter-- <= 0) {
                stopModelling();
                return false;
            }
        }

        LOGGER.info("work cycle...");

        last_cycle_length = millis() - last_cycle_time;

        last_cycle_time = millis();

        last_cycle_length = 3001;
        sensorController->fire();

        LOGGER.info(" last_cycle_length = " + String(last_cycle_length));

        calcMainCoreCharacteristics(last_cycle_length);

        handleModes();

        handlePumps();

        collectTelemetry(last_cycle_length);

        if (mode == STAND_BY) {
            if (time_to_close_oxygen_door_in_stanby_mode
                && (millis() > time_to_close_oxygen_door_in_stanby_mode)) {
                LOGGER.info("Oxygen door for closed.");
                time_to_close_oxygen_door_in_stanby_mode = 0;
                doorsController->setOxygenDoorValue(0);
            }
        } else
            time_to_close_oxygen_door_in_stanby_mode = 0;
    }

    if (main_door_opened != previous_main_door_opened) {
        LOGGER.info("Main door status has changed to " + String(main_door_opened ? "open" : "closed"));
        doorsController->setDoopOpened(main_door_opened);
        previous_main_door_opened = main_door_opened;
    }

    return proceeded;

}
//-------------------------------------------------------------------


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
    entered_to_warming_mode_at_cycle_index = cycle_index;
    pumpsController->setOnPumpsCount(2);
    doorsController->setSmokePipeValue(heaterSettings->warming_settings.smoke_door_value_prcnt);
    doorsController->setOxygenDoorValue(100);
    doorsController->setUpperDoorValue(heaterSettings->warming_settings.upper_door_value_prcnt);
    resetDEMAtimers();
}
//-------------------------------------------------------------------

/**
 * handling WARMING mode
 */
void HeaterController::handle_WARMING_mode() {
    // check if power reached target power to switch to PID mode
//    if ((mainCoreParams.core_EMA_power >= heaterSettings->warming_settings.target_power_to_switch_to_the_PID_mode)
//        && (sensorController->getSmaValue(T_SENS_INDEX_CORE) >
//            heaterSettings->warming_settings.start_pid_temperature))
    if (mainCoreParams.core_EMA_power >= heaterSettings->warming_settings.target_power_to_switch_to_the_PID_mode)
        switchTo_PID_mode();
    else
        // check if time to reach target power is up
    if (((millis() - entered_to_warming_mode_at) / 1000 >
         heaterSettings->warming_settings.time_to_reach_target_power_sec)
        || (modelling_is_active &&
            (((cycle_index - entered_to_warming_mode_at_cycle_index) * heaterSettings->scan_interval_ms / 1000.0)
             >= heaterSettings->warming_settings.time_to_reach_target_power_sec)))
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
                  < heaterSettings->finalCoolingSettings.delay_to_switch_to_standby_mode_sec)
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
    doorsController->setSmokePipeValue(heaterSettings->burning_settings.smoke_door_value_prcnt);
    doorsController->setUpperDoorValue(heaterSettings->burning_settings.upper_door_value_prcnt);
    resetDEMAtimers();
}
//-------------------------------------------------------------------

void HeaterController::handle_PID_mode() {
    // check for overheat mode
    if (sensorController->getSmaValue(T_SENS_INDEX_CORE)
        >= heaterSettings->temperatureSettings.core_overheat)
        switchTo_OVERHEATED_mode();
        // check for back to warming mode
    else if ((mainCoreParams.core_EMA_power < heaterSettings->burning_settings.power_to_switch_to_warming_mode)
             && (pidRegulator->getValuePrcnt() > heaterSettings->burning_settings.oxygen_door_val_to_warming_mode))
        switchTo_WARMING_mode();
    else {
//        pidRegulator->handle();
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

void HeaterController::startModelling() {
    if (!modelling_is_active) {
        LOGGER.info(" ---- STARTING MODELLING ----");
        coreModel->reset();
        switchTo_STAND_BY_mode();
        sensorController->setModeller(coreModel);
        estimated_modelling_cycle_counter = coreModel->calcTotalCycles();
        modelling_is_active = true;
        openOxygenDoorForTime(1200);
    } else
        LOGGER.error(" MODELLING ALREADY STARTED.");
}
//-------------------------------------------------------------------

void HeaterController::stopModelling() {
    if (modelling_is_active) {
        LOGGER.info(" ---- MODELLING FINISHED ----");
        LOGGER.info(" ---- MODELLING: estimated cycles = " + String(estimated_modelling_cycle_counter));
        sensorController->setModeller(nullptr);
        estimated_modelling_cycle_counter = 0;
        modelling_is_active = false;
    } else
        LOGGER.error(" MODELLING ALREADY STOPPED.");

}
//-------------------------------------------------------------------

bool HeaterController::isModelling() {
    return modelling_is_active;
}
//-------------------------------------------------------------------

bool HeaterController::isOxygenDoorOpenedForATime() {
    return ((time_to_close_oxygen_door_in_stanby_mode != 0)
        && (millis() < time_to_close_oxygen_door_in_stanby_mode));
}
//-------------------------------------------------------------------

TelemetryDataRecord *HeaterController::getTelemetryRecord() {
    return &telemetryDataRecord;
}
//-------------------------------------------------------------------

void HeaterController::getTelemetry(char *buffer) {
    int tToCloseOxygenDoor = !time_to_close_oxygen_door_in_stanby_mode ? 0 : (time_to_close_oxygen_door_in_stanby_mode - millis()) / 1000;
    if (tToCloseOxygenDoor < 0)
        tToCloseOxygenDoor = 0;

    sprintf(buffer, "{"
                    "\"date_time_ms\":\"%d\", \"interval_ms\": \"%d\","
                    "\"temperature\":{"
                    "\"core\": \"%00.2f\", \"input_t\": \"%00.2f\", "
                    "\"output_t\": \"%00.2f\", \"acc_top\": \"%00.2f\", "
                    "\"acc_upper\": \"%00.2f\", \"acc_low\": \"%00.2f\", "
                    "\"acc_bottom\": \"%00.2f\","
                    "\"forward_to_home\": \"%00.2f\", \"backward_from_home\": \"%00.2f\","
                    "\"core_sma_diff\":\"%00.2f\""
                    "}, "
                    "\"pumps\": {"
                    "\"pump_1_on\":%s, "
                    "\"pump_2_on\":%s, "
                    "\"core_flow\": \"%00.2f\" "
                    "}, "
                    "\"core_power\": \"%d\", "
                    "\"doors\":{"
                    "\"smoke\": \"%00.2f\", \"oxygen\": \"%00.2f\", "
                    "\"upper\": \"%00.2f\", "
                    "\"main_door\": %s, "
                    "\"oxygenManualTimer\": \"%i\" "
                    "},"
                    "\"pid\":{"
                    "\"p\":\"%00.2f\", \"i\":\"%00.2f\", \"d\":\"%00.2f\", "
                    "\"output\": \"%00.2f\""
                    "},"
                    "\"mode\":%d"
                    "}", telemetryDataRecord.date_time_ms, telemetryDataRecord.interval_ms, telemetryDataRecord.core_temp_sma, telemetryDataRecord.input_temp_sma,
            telemetryDataRecord.output_temp_sma, telemetryDataRecord.accumulator_top_temp_sma, telemetryDataRecord.accumulator_higher_temp_sma,
            telemetryDataRecord.accumulator_lower_temp_sma, telemetryDataRecord.accumulator_bottom_temp_sma,
            telemetryDataRecord.forwar_flow_temp_sma, telemetryDataRecord.backward_flow_temp_sma, telemetryDataRecord.core_SMA_diff_tempr,
            telemetryDataRecord.pump_1_state ? "true" : "false", telemetryDataRecord.pump_2_state ? "true" : "false",
            telemetryDataRecord.core_flow, (int)telemetryDataRecord.core_EMA_power, telemetryDataRecord.smoke_door_position,
            telemetryDataRecord.oxygen_door_position, telemetryDataRecord.upper_door_position, main_door_opened ? "true" : "false", tToCloseOxygenDoor, telemetryDataRecord.pid_p, telemetryDataRecord.pid_i,
            telemetryDataRecord.pid_d, telemetryDataRecord.pid_output, telemetryDataRecord.heaterMode);
}

