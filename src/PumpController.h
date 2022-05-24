//
// Created by dsporykhin on 24.05.22.
//

#ifndef BASE_ESP8266_MQTT_PUMPCONTROLLER_H
#define BASE_ESP8266_MQTT_PUMPCONTROLLER_H

enum StateName{
    MAN_OFF_S_OFF = 0,
    MAN_OFF_S_ON = 1,
    MAN_ON_S_OFF = 2,
    MAN_ON_S_ON = 3
};

class PumpController {
private:
    uint8_t pin;
    bool stateOn;
    bool manual_on;

    void stateToPin();
public:
    PumpController(uint8_t pin);
    bool isStateOn();
    bool setStateOn(bool new_stateOn);
    bool isManualOn();
    void setManualOn(bool manual_on);

    bool isOn();

    StateName getStateName();
};

#endif //BASE_ESP8266_MQTT_PUMPCONTROLLER_H
