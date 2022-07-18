//
// Created by dsporykhin on 17.07.22.
//

#ifndef BASE_ESP8266_MQTT_FLOWSENSOR_H
#define BASE_ESP8266_MQTT_FLOWSENSOR_H


class FlowSensor {
private:
    int pin;

    volatile int ticks;

    static IRAM_ATTR void ISR();
public:
    static FlowSensor *flowSensorInstance;

    FlowSensor(int pin);
    int readAndReset();
};


#endif //BASE_ESP8266_MQTT_FLOWSENSOR_H
