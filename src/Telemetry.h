//
// Created by dsporykhin on 03.05.22.
//

#ifndef BASE_ESP8266_MQTT_TELEMETRY_H
#define BASE_ESP8266_MQTT_TELEMETRY_H

#include <SD.h>
#include "SettingsManager.h"

#define TELEMETRY_BITS_FOR_BUFFER_SIZE 10
#define TELEMETRY_FILE_NAME "telemetry.csv"

struct TelemetryDataRecord{
    long date_time_ms;
    int interval_ms;
    float core_temp;
    float input_temp;
    float output_temp;
    float accumulator_bottom_temp;
    float accumulator_lower_temp;
    float accumulator_higher_temp;
    float accumulator_top_temp;
    float forwar_flow_temp;
    float backward_flow_temp;
    float avarage_backward_flow;

    bool pid_on_hold;
    float pid_d;
    float pid_prior_value;
    float pid_i;
    float pid_p;
    float pid_output;

    float oxygen_door_position;
    float smoke_door_position;
    float upper_door_position;

    bool pump_1_is_on;
    bool pump_2_is_on;
};

class Telemetry {
private:
    File data_file;
    long last_flush_time;

    TelemetryDataRecord *data;

    int index_ofnext;
    int mask_for_index;

    bool file_store_active;

    void flushDataFile();
    void openDataFile();

    GlobalSettings *settings;

    char *file_name;

public:
    Telemetry(GlobalSettings *settings);

    bool addData(TelemetryDataRecord* dataRecord);
};


#endif //BASE_ESP8266_MQTT_TELEMETRY_H
