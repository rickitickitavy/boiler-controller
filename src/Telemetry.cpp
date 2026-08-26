//
// Created by dsporykhin on 03.05.22.
//

#include "Telemetry.h"
#include "Logger.h"


Telemetry::Telemetry(GlobalSettings *settings) {
    this->settings = settings;

    LOGGER.info("Telemetry starting...");
    index_of_next = 0;
    mask_for_index = (1 << TELEMETRY_BITS_FOR_BUFFER_SIZE) - 1;
    // Ring buffer unused while addData memcpy is commented out — do not malloc ~100KB
    // before WiFi (esp_wifi_init ESP_ERR_NO_MEM / "Expected to init 4 rx buffer").
    data = nullptr;
    file_name = nullptr;
    file_store_active = false;
    last_flush_time = millis();
    stored_records_from_last_flush = 0;

    save_buffer = (char *) malloc(2048);
    sprintf(save_buffer, "mask %x", mask_for_index);
    LOGGER.info(save_buffer);
}

void Telemetry::sendCsvHeader() {
    char *header = "csv->;date_time_ms;interval_ms;internal_temp;core_temp;core_temp_sma;input_temp;"
            "input_temp_sma;output_temp;output_temp_sma;accumulator_bottom_temp;"
            "accumulator_bottom_temp_sma;accumulator_lower_temp;accumulator_lower_temp_sma;"
            "accumulator_higher_temp;"
            "accumulator_higher_temp_sma;accumulator_top_temp;accumulator_top_temp_sma;"
            "forwar_flow_temp;forwar_flow_temp_sma;backward_flow_temp;backward_flow_temp_sma;"
            "avarage_backward_flow;core_flow;core_power;core_EMA_power;pid_raw_output;"
            "pid_d;pid_i;pid_p;pid_output;oxygen_door_position;smoke_door_position;"
            "upper_door_position;pump_1_state;pump_2_state;heaterMode;core_SMA_diff_tempr;main_door_opened";
    Serial.println(header);

}

void Telemetry::flushDataFile() {
}

void Telemetry::openDataFile() {
    file_store_active = false;
}

void Telemetry::handleFlush() {
}

bool Telemetry::addData(TelemetryDataRecord *dataRecord) {
    //  memcpy(&data[index_of_next++ & mask_for_index], telemetryDataRecord, sizeof(TelemetryDataRecord));
    sprintf(save_buffer, "%d;%d;%f;"
                    "%f;%f;%f;%f;"
                    "%f;%f;"
                    "%f;%f;"
                    "%f;%f;"
                    "%f;%f;"
                    "%f;%f;"
                    "%f;%f;"
                    "%f;%f;"
                    "%f;%f;"
                    "%f;%f;"
                    "%f;"
                    "%f;%f;%f;%f;"
                    "%f;%f;"
                    "%f;"
                    "%d;%d;"
                    "%d;"
                    "%f;%s\r\n",
            dataRecord->date_time_ms, dataRecord->interval_ms, dataRecord->internal_temp,
            dataRecord->core_temp, dataRecord->core_temp_sma, dataRecord->input_temp, dataRecord->input_temp_sma,
            dataRecord->output_temp, dataRecord->output_temp_sma,
            dataRecord->accumulator_bottom_temp, dataRecord->accumulator_bottom_temp_sma,
            dataRecord->accumulator_lower_temp, dataRecord->accumulator_lower_temp_sma,
            dataRecord->accumulator_higher_temp, dataRecord->accumulator_higher_temp_sma,
            dataRecord->accumulator_top_temp, dataRecord->accumulator_top_temp_sma,
            dataRecord->forwar_flow_temp, dataRecord->forwar_flow_temp_sma,
            dataRecord->backward_flow_temp, dataRecord->backward_flow_temp_sma,
            dataRecord->avarage_backward_flow, dataRecord->core_flow,
            dataRecord->core_power, dataRecord->core_EMA_power,

            dataRecord->pid_raw_output,
            dataRecord->pid_d, dataRecord->pid_i, dataRecord->pid_p, dataRecord->pid_output,
            dataRecord->oxygen_door_position, dataRecord->smoke_door_position,
            dataRecord->upper_door_position,
            dataRecord->pump_1_state, dataRecord->pump_2_state,
            dataRecord->heaterMode,
            dataRecord->core_SMA_diff_tempr,
            dataRecord->main_door_opened ? "true" : "false");
    Serial.print("csv->;");
    Serial.println(save_buffer);
    if ((save_buffer) && settings->telemetrySettings.log_gebug_to_UART) {
        sprintf(save_buffer, "----------\r\n time ms = %d; interval ms = %d\r\n"
                        " internal t = %f \r\n"
                        " core t = %f (%f); inp t = %f (%f) \r\n"
                        "out t = %f (%f)\r\n"
                        " acc_b t = %f (%f); acc_l t = %f (%f)\r\n"
                        " acc_h t = %f (%f); acc_t t = %f (%f)\r\n"
                        " forwd_fl t = %f (%f); backwrd_fl t = %f (%f)\r\n"
                        " avg_bckwrd_fl = %f; core_fl = %f\r\n"
                        " core_pwr = %f; core_EMA_pwr = %f\r\n"
                        " pid_p = %f; pid_i = %f; pid_d = %f\r\n"
                        " pid_pid_raw_out = %f; pid_out = %f\r\n"
                        " oxy_dr_p = %f; smk_dr_p = %f; upper_dr_p = %f\r\n"
                        " pump_1 = %d; pump_2 = %d\r\n"
                        " heaterMode = %d\r\n"
                        " core_SMA_df t = %f\r\n"
                        " main door opened = %s\r\n------------",
                dataRecord->date_time_ms, dataRecord->interval_ms, dataRecord->internal_temp,
                dataRecord->core_temp, dataRecord->core_temp_sma, dataRecord->input_temp, dataRecord->input_temp_sma,
                dataRecord->output_temp, dataRecord->output_temp_sma,
                dataRecord->accumulator_bottom_temp, dataRecord->accumulator_bottom_temp_sma,
                dataRecord->accumulator_lower_temp, dataRecord->accumulator_lower_temp_sma,
                dataRecord->accumulator_higher_temp, dataRecord->accumulator_higher_temp_sma,
                dataRecord->accumulator_top_temp, dataRecord->accumulator_top_temp_sma,
                dataRecord->forwar_flow_temp, dataRecord->forwar_flow_temp_sma, dataRecord->backward_flow_temp,
                dataRecord->backward_flow_temp_sma,
                dataRecord->avarage_backward_flow, dataRecord->core_flow,
                dataRecord->core_power, dataRecord->core_EMA_power,
                dataRecord->pid_p, dataRecord->pid_i, dataRecord->pid_d,
                dataRecord->pid_raw_output, dataRecord->pid_output,
                dataRecord->oxygen_door_position, dataRecord->smoke_door_position, dataRecord->upper_door_position,
                dataRecord->pump_1_state, dataRecord->pump_2_state,
                dataRecord->heaterMode,
                dataRecord->core_SMA_diff_tempr, dataRecord->main_door_opened ? "true" : "false");
        LOGGER.info(save_buffer);
    }
    return false;
}

void Telemetry::getRawCsvSensors(TelemetryDataRecord *dataRecord, char *buffer) {
    sprintf(buffer, " %0.3f;%0.3f;%0.3f;%0.3f;%0.3f;%0.3f;%0.3f;%0.3f;%0.3f;%0.1f",
            dataRecord->internal_temp, dataRecord->core_temp
            , dataRecord->input_temp, dataRecord->output_temp
            , dataRecord->accumulator_bottom_temp, dataRecord->accumulator_lower_temp
            , dataRecord->accumulator_higher_temp, dataRecord->accumulator_top_temp
            , dataRecord->forwar_flow_temp, dataRecord->core_flow);
}

void Telemetry::getCsvCalculates(TelemetryDataRecord *dataRecord, char *buffer) {
    sprintf(buffer, "%d;%d;%0.2f;"
            "%d;%0.2f;%0.2f;"
            "%s"
            , dataRecord->smoke_door_position, dataRecord->upper_door_position
            , dataRecord->oxygen_door_position

            , dataRecord->core_power, dataRecord->power_balance_ready ? dataRecord->power_balance_kwt_hour : NAN
            , dataRecord->accumulated_energy_kwt_hour

            , dataRecord->pump_1_state | dataRecord->pump_1_state ? "ON" : "OFF"
    );
}

void Telemetry::getCsvSensors(TelemetryDataRecord *dataRecord, char *buffer) {
    sprintf(buffer, " %0.2f;%0.2f;%0.2f;%0.2f;%0.2f;%0.2f;%0.2f;%0.2f;%0.2f;%0.1f;",
            dataRecord->internal_temp, dataRecord->core_temp_sma
            , dataRecord->input_temp_sma, dataRecord->output_temp_sma
            , dataRecord->accumulator_bottom_temp_sma, dataRecord->accumulator_lower_temp_sma
            , dataRecord->accumulator_higher_temp_sma, dataRecord->accumulator_top_temp_sma
            , dataRecord->forwar_flow_temp_sma, dataRecord->core_flow);
}

