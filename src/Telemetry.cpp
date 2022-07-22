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
    data = (TelemetryDataRecord *) malloc((1 << TELEMETRY_BITS_FOR_BUFFER_SIZE - 1) * sizeof(TelemetryDataRecord));

    char _cat_name[128];
    sprintf(_cat_name, "/%s", settings->telemetrySettings.catName);

    if (LOGGER.isSdPresents()) {
        if (!SD.exists(_cat_name)) {
            LOGGER.info("Creating catalog for storing data...");
            if (!SD.mkdir(_cat_name))
                LOGGER.error("Error creating catalog  for telemetry");
            else
                LOGGER.error("Catalog for telemetry created");
        }

        file_name = (char *) malloc(strlen(settings->telemetrySettings.catName)
                                    + strlen(TELEMETRY_FILE_NAME)
                                    + 3);
        sprintf(file_name, "/%s/%s", settings->telemetrySettings.catName, TELEMETRY_FILE_NAME);

        LOGGER.info(file_name);

        last_flush_time = millis();
        stored_records_from_last_flush = 0;

        openDataFile();
    } else {
        file_store_active = false;
    }

    save_buffer = (char *) malloc(1024);
    sprintf(save_buffer, "mask %x", mask_for_index);
    LOGGER.info(save_buffer);
}

void Telemetry::sendCsvHeader(){
    char *header = "csv->;date_time_ms;interval_ms;core_temp;core_temp_sma;input_temp;"
            "input_temp_sma;output_temp;output_temp_sma;accumulator_bottom_temp;"
            "accumulator_bottom_temp_sma;accumulator_lower_temp;accumulator_lower_temp_sma;"
            "accumulator_higher_temp;"
            "accumulator_higher_temp_sma;accumulator_top_temp;accumulator_top_temp_sma;"
            "forwar_flow_temp;forwar_flow_temp_sma;backward_flow_temp;backward_flow_temp_sma;"
            "avarage_backward_flow;core_flow;core_power;core_EMA_power;pid_raw_output;"
            "pid_d;pid_i;pid_p;pid_output;oxygen_door_position;smoke_door_position;"
            "upper_door_position;pump_1_state;pump_2_state;heaterMode;core_SMA_diff_tempr";
    Serial.println(header);

}

void Telemetry::flushDataFile() {
    data_file.flush();
    data_file.close();
}

void Telemetry::openDataFile() {
    bool _new_file = !SD.exists(file_name);

    data_file = SD.open(file_name, FILE_APPEND);
    // check for file needs to be move to archive
    size_t f_size = data_file.size();
    if ((f_size < 1000000000) && (f_size > settings->telemetrySettings.max_file_size_bytes)) {
        // rename current file to archive
        data_file.close();

        char _long_buf[17];
        sprintf(_long_buf, "%i", millis());

        char *_arch_file_name = (char *) malloc(strlen(settings->telemetrySettings.catName)
                                                + strlen(_long_buf)
                                                + strlen(TELEMETRY_FILE_NAME)
                                                + 4);
        sprintf(_arch_file_name, "/%s/%s-%s", settings->telemetrySettings.catName, _long_buf, TELEMETRY_FILE_NAME);
        LOGGER.info(_arch_file_name);
        if (!SD.rename(file_name, _arch_file_name))
            LOGGER.error("error move telemetry file to archive");
        else {
            LOGGER.info("telemetry file moved to archive");
            data_file = SD.open(file_name, FILE_APPEND);
            _new_file = true;
        }
    }

    char *header = "date_time_ms;interval_ms;core_temp;core_temp_sma;input_temp;"
            "input_temp_sma;output_temp;output_temp_sma;accumulator_bottom_temp;"
            "accumulator_bottom_temp_sma;accumulator_lower_temp;accumulator_lower_temp_sma;"
            "accumulator_higher_temp;"
            "accumulator_higher_temp_sma;accumulator_top_temp;accumulator_top_temp_sma;"
            "forwar_flow_temp;forwar_flow_temp_sma;backward_flow_temp;backward_flow_temp_sma;"
            "avarage_backward_flow;core_flow;core_power;core_EMA_power;pid_raw_output;"
            "pid_d;pid_i;pid_p;pid_output;oxygen_door_position;smoke_door_position;"
            "upper_door_position;pump_1_state;pump_2_state;heaterMode;core_SMA_diff_tempr\r\n";
    if (data_file) {
        file_store_active = true;
        stored_records_from_last_flush = 0;
        last_flush_time = millis();
        if (_new_file){
            data_file.write((uint8_t*)header, strlen(header));
        }
    }
}

void Telemetry::handleFlush() {
    if ((((millis() - last_flush_time) > settings->telemetrySettings.flush_interval_ms) &
         (stored_records_from_last_flush > 0))
        || (stored_records_from_last_flush > settings->telemetrySettings.flush_inteval_records)) {
        LOGGER.info("Telemetry flushing...");
        flushDataFile();
        openDataFile();
    }
}

bool Telemetry::addData(TelemetryDataRecord *dataRecord) {
  //  memcpy(&data[index_of_next++ & mask_for_index], telemetryDataRecord, sizeof(TelemetryDataRecord));
        sprintf(save_buffer, "%d;%d;"
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
                        "%f\r\n",
                dataRecord->date_time_ms, dataRecord->interval_ms,
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
                dataRecord->core_SMA_diff_tempr);
    Serial.print("csv->;");
    Serial.println(save_buffer);
    if (file_store_active) {
        if (!data_file.write((uint8_t *) save_buffer, strlen(save_buffer))) {
            LOGGER.error("Error writing to telemetry file");
            return true;
        } else {
            stored_records_from_last_flush++;
            handleFlush();
            return true;
        }
    } else if ((save_buffer) && settings->telemetrySettings.log_gebug_to_UART){
        sprintf(save_buffer, "----------\r\n time ms = %d; interval ms = %d\r\n"
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
                        " core_SMA_df t = %f\r\n------------",
                dataRecord->date_time_ms, dataRecord->interval_ms,
                dataRecord->core_temp, dataRecord->core_temp_sma, dataRecord->input_temp, dataRecord->input_temp_sma,
                dataRecord->output_temp, dataRecord->output_temp_sma,
                dataRecord->accumulator_bottom_temp, dataRecord->accumulator_bottom_temp_sma, dataRecord->accumulator_lower_temp, dataRecord->accumulator_lower_temp_sma,
                dataRecord->accumulator_higher_temp, dataRecord->accumulator_higher_temp_sma, dataRecord->accumulator_top_temp, dataRecord->accumulator_top_temp_sma,
                dataRecord->forwar_flow_temp, dataRecord->forwar_flow_temp_sma, dataRecord->backward_flow_temp, dataRecord->backward_flow_temp_sma,
                dataRecord->avarage_backward_flow, dataRecord->core_flow,
                dataRecord->core_power, dataRecord->core_EMA_power,
                dataRecord->pid_p, dataRecord->pid_i, dataRecord->pid_d,
                dataRecord->pid_raw_output, dataRecord->pid_output,
                dataRecord->oxygen_door_position, dataRecord->smoke_door_position, dataRecord->upper_door_position,
                dataRecord->pump_1_state, dataRecord->pump_2_state,
                dataRecord->heaterMode,
                dataRecord->core_SMA_diff_tempr);
        LOGGER.info(save_buffer);
    }
}

