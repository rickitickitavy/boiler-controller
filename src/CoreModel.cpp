//
// Created by dsporykhin on 27.06.22.
//

#include <math.h>
#include <Arduino.h>
#include "CoreModel.h"
#include "DoorsController.h"

void moveWarmedWater(double t_moved, double v_moved, double t2, double v2) {

}

void CoreModel::reset() {
    bottom_tempr = 25;
    lower_tempr = 25;
    higher_tempr = 25;
    top_tempr = 25;
    core_volume_temp = 25;
    core_tempr = 25;
    output_tempr = 25;
    core_heated = false;
    max_core_power_reached = false;

    angle = 0;
    current_core_power_doors_coef = 0;

    angle_step = settings->modellerSettings.core_power_grow_angle_step;
    skip_steps = settings->modellerSettings.skip_first_N_cycles;
    max_power = settings->modellerSettings.max_core_power;
    additioanl_core_doors_coef = settings->modellerSettings.additional_core_doors_coef;
    cycle_index = 0;

    radiator_normal_temperature = settings->modellerSettings.radiators_base_temperature;
    radiator_normal_power = settings->modellerSettings.radiators_base_power;
    home_temperature = settings->modellerSettings.radiators_home_temperature;
    radiator_flow = settings->modellerSettings.radiators_flow_lpm;
    radiator_pow_coef = settings->modellerSettings.radiators_efficiensy_coef_pow;

    doors_react_ema = settings->modellerSettings.ema_doors_reactions;
    max_power_intervals_estimated = settings->modellerSettings.max_power_cycles;
    energy_move_coef = settings->modellerSettings.core_energy_transmitting_coef;

    radiator_stop_after_cycle = settings->modellerSettings.radiator_stop_after_cycle;

    telemetry->sendCsvHeader();
    max_power_intervals_estimated = settings->modellerSettings.max_power_cycles;
}

void CoreModel::handle() {
    if (skip_steps) {
        skip_steps--;
        return;
    }

    println("coreModel: ========================== " + String(cycle_index++));

    // power is growing ?
    if (angle < 90) {
        angle += angle_step;
        if (angle > 90)
            angle = 90;
    } else if (!max_core_power_reached) {
        power_fade_out_steps = settings->modellerSettings.power_fade_out_steps;
        max_power_intervals_estimated = settings->modellerSettings.max_power_cycles;
        max_core_power_reached = true;
        println("coreModel: max power reached for " + String(max_power_intervals_estimated) + " cycles");
    }

    current_power = max_power * sin(angle * PI / 180);

    double _target_core_power_doors_coef = (doorsController->getOxygenDoorAngle() /
                                            settings->servos_hardware_settings.oxygen_servo_settings.working_max_angle) *
                                           sin(doorsController->getSmokePipeAngle() / 180 * PI) *
                                           additioanl_core_doors_coef;
    println("coreModel: _target_core_power_doors_coef = " + String(_target_core_power_doors_coef));

    current_core_power_doors_coef = current_core_power_doors_coef / doors_react_ema * (doors_react_ema - 1)
                                    + _target_core_power_doors_coef / doors_react_ema;
    println("coreModel: core_power_doors_coef = " + String(current_core_power_doors_coef));

    current_power *= current_core_power_doors_coef;

    if (current_power > max_power)
        current_power = max_power;

    if ((current_power > 10000) && !core_heated) {
        println("coreModel: core heated !!!");
        core_heated = true;
    }

    if ((current_power < 10000) && core_heated)
        current_power = 10000;

    if (doorsController->getOxygenDoorAngle() == 0) {
        double _cooling_core_coef = sin(doorsController->getUpperDoorAngle() / 180 * PI) *
                                    sin(doorsController->getSmokePipeAngle() / 180 * PI) * 2;
        println("coreModel: _cooling_core_coef = " + String(_cooling_core_coef));

        current_power -= 8000 * _cooling_core_coef * (core_tempr - 25) / core_tempr;
    }

    if (max_core_power_reached) {
        if (max_power_intervals_estimated > 0)
            max_power_intervals_estimated--;

        if (max_power_intervals_estimated == 0) {
            current_power *= (double) power_fade_out_steps
                             / (double) settings->modellerSettings.power_fade_out_steps;
            if (power_fade_out_steps >= 0)
                power_fade_out_steps--;
        }
    }

    println(" power = " + String(current_power));
    println(" energy = " + String(settings->scan_interval_ms * current_power / 1000));


    // warming core. NOT REAL!!!
    core_tempr += settings->scan_interval_ms * current_power / 1000 / core_energy_volume_dg_per_grad;
    println("coreModel: core_tempr new = " + String(core_tempr));

    // warm water by warmed core
    // if pumps is on - move warmed water to top of accumulator
    if (pumpsController->getOnPumpsCount()) {
        println("coreModel: PUMPS ARE ON");
        double _warmed_volume = 0;
        if (pumpsController->pump1->isOn())
            _warmed_volume += settings->two_pumps_settings.first_pump_flow_litters_per_minute;
        if (pumpsController->pump2->isOn())
            _warmed_volume += settings->two_pumps_settings.second_pump_flow_litters_per_minute;
        _warmed_volume = _warmed_volume / 60 * settings->scan_interval_ms / 1000;

        println("coreModel: _warmed_volume = " + String(_warmed_volume));

        double _acc_part_vol = settings->capacities_setting.accumulator_ltr / 4 - _warmed_volume;
        println("coreModel: _acc_part_vol = " + String(_acc_part_vol));

        double _delta_tempr_acc_part = _warmed_volume / _acc_part_vol * (lower_tempr - bottom_tempr);
        println("coreModel: _delta_tempr_acc_part = " + String(_delta_tempr_acc_part));
        bottom_tempr += _delta_tempr_acc_part;
        println("coreModel: bottom_tempr = " + String(bottom_tempr));

        _delta_tempr_acc_part = _warmed_volume / _acc_part_vol * (higher_tempr - lower_tempr);
        println("coreModel: _delta_tempr_acc_part = " + String(_delta_tempr_acc_part));
        lower_tempr += _delta_tempr_acc_part;
        println("coreModel: lower_tempr = " + String(lower_tempr));

        _delta_tempr_acc_part = _warmed_volume / _acc_part_vol * (top_tempr - higher_tempr);
        println("coreModel: _delta_tempr_acc_part = " + String(_delta_tempr_acc_part));
        higher_tempr += _delta_tempr_acc_part;
        println("coreModel: higher_tempr = " + String(higher_tempr));

        // delta in energy between core and warmed water
        println("coreModel: core_volume_temp = " + String(core_volume_temp));
        double _delta_energy = (core_tempr - core_volume_temp) * core_energy_volume_dg_per_grad;
        println("coreModel: _delta_energy = " + String(_delta_energy));
        // energy you need to heat core water per 1 grad
        double _core_vol_energy_dg_per_grad = 4200 * settings->capacities_setting.heater_core_ltr;
        println("coreModel: _core_vol_energy_dg_per_grad = " + String(_core_vol_energy_dg_per_grad));
        // average temperature
        double _avg_delta_tempr = _delta_energy / (_core_vol_energy_dg_per_grad + core_energy_volume_dg_per_grad);
        println("coreModel: _avg_delta_tempr = " + String(_avg_delta_tempr));

        double _moved_core_energy = _avg_delta_tempr * core_energy_volume_dg_per_grad * energy_move_coef;
        println("coreModel: _moved_core_energy = " + String(_moved_core_energy));

        core_tempr -= _moved_core_energy / core_energy_volume_dg_per_grad;
        println("coreModel: core_tempr = " + String(core_tempr));
        core_volume_temp += _moved_core_energy / (settings->capacities_setting.heater_core_ltr * 4200);
        println("coreModel: core_volume_temp = " + String(core_volume_temp));
        output_tempr = core_volume_temp;
        println("coreModel: output_tempr = " + String(output_tempr));

        // water from core to accumulator
        _delta_tempr_acc_part = _warmed_volume / _acc_part_vol * (core_volume_temp - top_tempr);
        top_tempr += _delta_tempr_acc_part;
        println("coreModel: top_tempr = " + String(top_tempr));

        // new core vol tempr
        core_volume_temp += _warmed_volume / (settings->capacities_setting.heater_core_ltr - _warmed_volume)
                            * (bottom_tempr - core_volume_temp);
        println("coreModel: core_volume_temp = " + String(core_volume_temp));

        if (cycle_index < radiator_stop_after_cycle) {

            // heating radiators
            double _energy_max = radiator_normal_power * settings->scan_interval_ms / 1000;
            println("coreModel: _energy_max = " + String(_energy_max));

            double _radiator_volume = radiator_flow / 60 * settings->scan_interval_ms / 1000;
            println("coreModel: _radiator_volume = " + String(_radiator_volume));

            double _tempr_coef = pow((top_tempr - home_temperature) / (radiator_normal_temperature - home_temperature),
                                     radiator_pow_coef);
            println("coreModel: _tempr_coef = " + String(_tempr_coef));

            double _radiator_energy = _energy_max * _tempr_coef;
            println("coreModel: _radiator_energy = " + String(_radiator_energy));

            double _after_radiator_tempr = top_tempr - _radiator_energy / 4200 / _radiator_volume;
            println("coreModel: _after_radiator_tempr = " + String(_after_radiator_tempr));

            _acc_part_vol = settings->capacities_setting.accumulator_ltr / 4 - _radiator_volume;
            _delta_tempr_acc_part = _radiator_volume / _acc_part_vol * (_after_radiator_tempr - bottom_tempr);
            println("coreModel: _delta_tempr_acc_part = " + String(_delta_tempr_acc_part));
            bottom_tempr += _delta_tempr_acc_part;
            println("coreModel: bottom_tempr = " + String(bottom_tempr));

            _delta_tempr_acc_part = _radiator_volume / _acc_part_vol * (bottom_tempr - lower_tempr);
            println("coreModel: _delta_tempr_acc_part = " + String(_delta_tempr_acc_part));
            lower_tempr += _delta_tempr_acc_part;
            println("coreModel: lower_tempr = " + String(lower_tempr));

            _delta_tempr_acc_part = _radiator_volume / _acc_part_vol * (lower_tempr - higher_tempr);
            println("coreModel: _delta_tempr_acc_part = " + String(_delta_tempr_acc_part));
            higher_tempr += _delta_tempr_acc_part;
            println("coreModel: higher_tempr = " + String(higher_tempr));

            _delta_tempr_acc_part = _radiator_volume / _acc_part_vol * (higher_tempr - top_tempr);
            println("coreModel: _delta_tempr_acc_part = " + String(_delta_tempr_acc_part));
            top_tempr += _delta_tempr_acc_part;
            println("coreModel: top_tempr = " + String(top_tempr));
        }
    } else {
        // pumps is off. warming heater volume
        println("coreModel: PUMPS ARE OFF");

        // delta in energy between core and warmed water
        double _delta_energy = (core_tempr - core_volume_temp) * core_energy_volume_dg_per_grad;
        println("coreModel: _delta_energy = " + String(_delta_energy));
        // energy you need to heat core water per 1 grad
        double _core_vol_energy_dg_per_grad = 4200 * settings->capacities_setting.heater_core_ltr;
        println("coreModel: _core_vol_energy_dg_per_grad = " + String(_core_vol_energy_dg_per_grad));
        // average temperature
        double _avg_delta_tempr = _delta_energy / (_core_vol_energy_dg_per_grad + core_energy_volume_dg_per_grad);
        println("coreModel: _avg_delta_tempr = " + String(_avg_delta_tempr));

        double _moved_core_energy = _avg_delta_tempr * core_energy_volume_dg_per_grad * energy_move_coef;
        println("coreModel: _moved_core_energy = " + String(_moved_core_energy));
        core_volume_temp += _moved_core_energy / (settings->capacities_setting.heater_core_ltr * 4200);
        println("coreModel: core_volume_temp = " + String(core_volume_temp));
    }
    Serial.println("coreModel: ---------------------- ");
}

CoreModel::CoreModel(HeaterSettings *settings, PumpsController *pumpsController, DoorsController *doorsController,
                     Telemetry *telemetry) {
    this->settings = settings;
    this->pumpsController = pumpsController;
    this->doorsController = doorsController;
    this->telemetry = telemetry;

    core_energy_volume_dg_per_grad = 460 * 220;

    fuel = 10000000;

    reset();
}

void CoreModel::println(String data) {
    if (settings->modellerSettings.logging_modeller_info)
        Serial.println(data);
}
