//
// Created by dsporykhin on 25.03.22.
//

#ifndef BASE_ESP8266_MQTT_SERVO_H
#define BASE_ESP8266_MQTT_SERVO_H
#define PWM_FREQUENCY 50.0


#include <cstdint>

class Servo {
private:
    uint8_t channel;
    uint8_t pin;
    int position;
    double angle_grad;
    double ticks_per_grad;
    double min_pulse_ticks;
    int min_pulse_length_us;
    int max_pulse_length_us;
    int rotation_grad;

    void calc_ticks_per_grad();
public:
    Servo(uint8_t pin, uint8_t channel);
    Servo* setAngle(double angle);
    Servo* setMinPulseLengthUs(int min_pulse_length_us);
    Servo* setMaxPulseLengthUs(int max_pulse_length_us);
    Servo* set_rotation_grad(int rotation_grad);

    double get_angle_grad();
};


#endif //BASE_ESP8266_MQTT_SERVO_H
