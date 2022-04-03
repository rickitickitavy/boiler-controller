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
 *      i_value
 *      d_value
 *      max_i_value
 *      min_absolute_output_value   - максимум значения,
 *      output_dynamic_diapason     - диапазон изменения выходного значения
 *
 *    smoke_pid_settings
 *      p_value
 *      i_value
 *      d_value
 *      max_i_value
 *      min_absolute_output_value   - максимум значения,
 *      output_dynamic_diapason     - диапазон изменения выходного значения
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


 HeaterController::HeaterController(GlobalSettings *settings,
                                   SensorController *sensorController){
    this->settings = settings;
    this->sensorController = sensorController;

    smoke_pipe_control = new Servo(SMOKE_SERVO_PIN, 0);
    oxygen_door_control = new Servo(OXYGEN_SERVO_PIN, 1);
    upper_door_control = new Servo(UPPER_SERVO_PIN, 2);
}
