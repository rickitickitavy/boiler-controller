//
// Created by dsporykhin on 08.03.22.
//

#ifndef BASE_ESP8266_MQTT_SENSORCONTROLLER_H
#define BASE_ESP8266_MQTT_SENSORCONTROLLER_H


#include <OneWire.h>
#include <DallasTemperature.h>
#include <lib/math/Sma.h>
#include "SettingsManager.h"
#ifdef ENABLE_MODELLING
#include "CoreModel.h"
#endif

enum SensorControllerStatus{
    STATUS_CYCLE_AWAITING, STATUS_CYCLE_STARTED, STATUS_CYCLE_DONE
};

struct SensorData{
    double value;
    bool data_ready;
    long last_time_read;
    int total_errors_count;
    long total_success_count;
    bool error_state;
    int last_state_count;
};

class SensorController {
private:

//    OneWire *oneWire;
//    OneWire *oneWire2;
    DallasTemperature *dallasTemperature;
    DallasTemperature *dallasTemperature2;
    SettingsManager *settingsManager;

    long last_time_sensors_read;
    bool hasSensors;

    bool hasData(char *data, int size);
#ifdef ENABLE_MODELLING
    void saveModelledSensorValue(int sensor_index, double value);
#endif

    Sma **smaSensors;

#ifdef ENABLE_MODELLING
    CoreModel *coreModel;
#endif

public:
    bool data_ready;
    uint8_t found_sensors_addr[SENSORS_ADDR_SIZE * MAX_SENSORS_COUNT];
    int found_sensors_count;
    SensorData sensor_data[MAX_SENSORS_COUNT];

    SensorControllerStatus cycle_status;
    int cycle_current_dallas_block;
    long cycle_conversion_started_at;
    DallasTemperature* cycle_dallas_temperature;

    SensorController(int one_wire_pin, int one_wire_pin_2, SettingsManager *settingsManager);
    int countSensors(DallasTemperature *dallasTemperature);
//    void readDallas(DallasTemperature *dallasTemperature);
    bool readDallasSensor(int sensor_index);
    void startAsyncConversion();
    void readNextBlockOfSensors();
#ifdef ENABLE_MODELLING
    void setModeller(CoreModel *coreModel);
#endif
    void handle();
    void fire();
    bool isHasSensors();
    float getSmaValue(int sensorIndex);
    float getNotNANSmaValue(int sensorIndex);
    float getNotNANValue(int sensorIndex);
    String buildSensorsList();
};


#endif //BASE_ESP8266_MQTT_SENSORCONTROLLER_H
