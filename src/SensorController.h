//
// Created by dsporykhin on 08.03.22.
//

#ifndef BASE_ESP8266_MQTT_SENSORCONTROLLER_H
#define BASE_ESP8266_MQTT_SENSORCONTROLLER_H


#include "DallasTemperature.h"
#include "SettingsManager.h"

struct SensorData{
    double value;
    bool data_ready;
    long last_time_read;
};

class SensorController {
private:

    OneWire *oneWire;
    DallasTemperature *dallasTemperature;
    SettingsManager *settingsManager;

    long last_time_sensors_read;
    bool hasSensors;

    bool hasData(char *data, int size);

public:
    bool data_ready;
    SensorData sensor_data[MAX_SENSORS_COUNT];

    SensorController(int one_wire_pin, SettingsManager *settingsManager);
    void handle();
    bool isHasSensors();
};


#endif //BASE_ESP8266_MQTT_SENSORCONTROLLER_H
