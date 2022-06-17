//
// Created by dsporykhin on 25.03.22.
//

#include <esp32-hal-gpio.h>
#include <Logger.h>
#include "Servo.h"


Servo::Servo(uint8_t pin, uint8_t channel){
    this->channel = channel;
    this->pin = pin;

    pinMode(pin, OUTPUT);
    ledcSetup(channel, PWM_FREQUENCY, 16);
    ledcAttachPin(pin, channel);

    min_pulse_length_us = 800;
    max_pulse_length_us = 2000;
    rotation_grad = 180;
    calc_ticks_per_grad();
}

Servo* Servo::setAngle(double angle){
    angle_grad = angle;
    if (angle_grad < 0)
        angle_grad = 0;
    else if (angle_grad > rotation_grad)
        angle_grad = rotation_grad;

    position = min_pulse_ticks + angle_grad * ticks_per_grad;
    LOGGER.info("Servo::setAngle 1");
    ledcWrite(channel, position);
    LOGGER.info("Servo::setAngle 2");
    return this;
}

void Servo::calc_ticks_per_grad(){
    double us_per_tick = 1.0 / PWM_FREQUENCY / 65535;
    double dynamic_depth = max_pulse_length_us - min_pulse_length_us;
    double us_per_grad = dynamic_depth / rotation_grad;
    ticks_per_grad = us_per_grad / us_per_tick;
    min_pulse_ticks = min_pulse_length_us / us_per_tick;
}

Servo* Servo::set_min_pulse_length_us(int min_pulse_length_us){
    this->min_pulse_length_us = min_pulse_length_us;
    calc_ticks_per_grad();
    return this;
}

Servo* Servo::set_max_pulse_length_us(int max_pulse_length_us){
    this->max_pulse_length_us = max_pulse_length_us;
    calc_ticks_per_grad();
    return this;
}

Servo* Servo::set_rotation_grad(int rotation_grad){
    this->rotation_grad = rotation_grad;
    calc_ticks_per_grad();
    return this;
}

double Servo::get_angle_grad(){
    return angle_grad;
}
