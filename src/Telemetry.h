//
// Created by dsporykhin on 03.05.22.
//

#ifndef BASE_ESP8266_MQTT_TELEMETRY_H
#define BASE_ESP8266_MQTT_TELEMETRY_H

#include <SD.h>
#include "SettingsManager.h"

#define TELEMETRY_BITS_FOR_BUFFER_SIZE 9
#define TELEMETRY_FILE_NAME "telemetry.csv"

struct TelemetryDataRecord{
    long date_time_ms;
    int interval_ms;
    float internal_temp;
    float core_temp;
    float input_temp;
    float output_temp;
    float accumulator_bottom_temp;
    float accumulator_lower_temp;
    float accumulator_higher_temp;
    float accumulator_top_temp;
    float forwar_flow_temp;
    float backward_flow_temp;
    float core_temp_sma;
    float input_temp_sma;
    float output_temp_sma;
    float accumulator_bottom_temp_sma;
    float accumulator_lower_temp_sma;
    float accumulator_higher_temp_sma;
    float accumulator_top_temp_sma;
    float forwar_flow_temp_sma;
    float backward_flow_temp_sma;

    float avarage_backward_flow;
    float core_flow;

    float core_power;
    float core_EMA_power;

    float pid_d;
    float pid_i;
    float pid_p;
    float pid_output;
    float pid_raw_output;

    float oxygen_door_position;
    float smoke_door_position;
    float upper_door_position;

    uint8_t pump_1_state;
    uint8_t pump_2_state;

    uint8_t heaterMode;
    float core_SMA_diff_tempr;
    bool main_door_opened;

    long time_to_close_oxygen_door_in_stanby_mode;

    float accumulated_energy_kwt_hour;
    float power_balance_kwt_hour;
    bool power_balance_ready;
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

    void sendCsvHeader();
};


#endif //BASE_ESP8266_MQTT_TELEMETRY_H
