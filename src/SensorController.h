//
// Created by dsporykhin on 08.03.22.
//

#ifndef BASE_ESP8266_MQTT_SENSORCONTROLLER_H
#define BASE_ESP8266_MQTT_SENSORCONTROLLER_H


#include <lib/oneWire/OneWire.h>
#include <lib/math/Sma.h>
#include "lib/dallasSensors/DallasTemperature.h"
#include "SettingsManager.h"
#include "CoreModel.h"

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
    void saveModelledSensorValue(int sensor_index, double value);

    Sma **smaSensors;

    CoreModel *coreModel;

public:
    bool data_ready;
    uint8_t found_sensors_addr[SENSORS_ADDR_SIZE * MAX_SENSORS_COUNT];
    int found_sensors_count;
    SensorData sensor_data[MAX_SENSORS_COUNT];

    SensorController(int one_wire_pin, SettingsManager *settingsManager);
    void setModeller(CoreModel *coreModel);
    void handle();
    void fire();
    bool isHasSensors();
    double getSmaValue(int sensorIndex);
    double getNotNANSmaValue(int sensorIndex);
    double getNotNANValue(int sensorIndex);
    String buildSensorsList();
};


#endif //BASE_ESP8266_MQTT_SENSORCONTROLLER_H
