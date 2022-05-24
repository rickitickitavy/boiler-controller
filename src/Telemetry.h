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
    float pid_i_sum;
    float pid_p;
    float pid_output;

    float oxygen_door_position;
    float smoke_door_position;
    float upper_door_position;

    uint8_t pump_1_state;
    uint8_t pump_2_state;

    uint8_t heaterMode;
    float core_SMA_diff_tempr;
};

class Telemetry {
private:
    File data_file;

    /**
     * date-time of last flushing and reopening telemetry file
     */
    long last_flush_time;

    int stored_records_from_last_flush;

    char *save_buffer;
    /**
     * time of last adding record to telemetry
     */
    long last_save_time_ms;

    TelemetryDataRecord *data;

    /**
     * index for next record. Index for real index in array calculates as index_of_next & mask_for_index
     */
    int index_of_next;
    int mask_for_index;

    bool file_store_active;

    void flushDataFile();
    void openDataFile();
    void handleFlush();

    GlobalSettings *settings;

    char *file_name;

public:
    Telemetry(GlobalSettings *settings);

    bool addData(TelemetryDataRecord* dataRecord);
};


#endif //BASE_ESP8266_MQTT_TELEMETRY_H
