//
// Created by dsporykhin on 08.03.21.
//

#ifndef BASE_ESP8266_MQTT_DS18B20_H
#define BASE_ESP8266_MQTT_DS18B20_H

#define MAX_DEVICES 64
#define DEVICE_OPTION_REPRESENTATION_LENGTH 16 + 4 + 6 + 7

#include <OneWire.h>

class DS18x20Hub {
private:
    OneWire *sensor;
    byte devices[MAX_DEVICES * 8];
    int deviceOptionRepresentationSize = DEVICE_OPTION_REPRESENTATION_LENGTH;
    char buffer[(DEVICE_OPTION_REPRESENTATION_LENGTH) * MAX_DEVICES + 1];

    int deviceCounter;
public:
    DS18x20Hub(int pin);

    int getDeviceCount();

    char *devicesToHTMLOptions();

    int readSensor(byte addr[]);
};


#endif //BASE_ESP8266_MQTT_DS18B20_H
