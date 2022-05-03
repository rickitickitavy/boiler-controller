//
// Created by dsporykhin on 03.05.22.
//

#include "Telemetry.h"
#include "Logger.h"


Telemetry::Telemetry(GlobalSettings *settings) {
    this->settings = settings;

    LOGGER.info("Telemetry starting...");
    index_ofnext = 0;
    mask_for_index = 2 ^ TELEMETRY_BITS_FOR_BUFFER_SIZE - 1;
    data = (TelemetryDataRecord *) malloc(2 ^ TELEMETRY_BITS_FOR_BUFFER_SIZE * sizeof(TelemetryDataRecord));


    if (LOGGER.isSdPresents()) {
        if (!SD.exists(settings->telemetrySettings.catName)) {
            LOGGER.info("Creating catalog for storing data...");
            if (!SD.mkdir(settings->telemetrySettings.catName))
                LOGGER.error("Error creating catalog  for telemetry");
            else
                LOGGER.error("Catalog for telemetry created");
        }

        file_name = (char *) malloc(strlen(settings->telemetrySettings.catName)
                                    + strlen(TELEMETRY_FILE_NAME)
                                    + 3);
        file_name[0] = '/';
        file_name[1] = 0;
        strcat(file_name, settings->telemetrySettings.catName);
        strcat(file_name, "/");
        strcat(file_name, TELEMETRY_FILE_NAME);
        LOGGER.info(file_name);
    } else {
        file_store_active = false;
    }
}

void Telemetry::flushDataFile() {
    if (file_store_active) {
        data_file.flush();
        data_file.close();
    }
}

void Telemetry::openDataFile() {
    data_file = SD.open(TELEMETRY_FILE_NAME);
    if (data_file.size() > settings->telemetrySettings.max_file_size_bytes) {
        // rename current file to archive
        data_file.close();

        char _long_buf[17];
        sprintf(_long_buf, "%i", millis());

        char *_arch_file_name = (char *) malloc(strlen(settings->telemetrySettings.catName)
                                                + strlen(_long_buf)
                                                + strlen(TELEMETRY_FILE_NAME)
                                                + 4);
        sprintf(_arch_file_name, "/%s/%s-%s"
                , settings->telemetrySettings.catName
        , _long_buf
        , TELEMETRY_FILE_NAME);
        LOGGER.info(_arch_file_name);
        if (!SD.rename(file_name,_arch_file_name))
            LOGGER.error("error move telemetry file to archive");
        else
            LOGGER.info("telemetry file moved to archive");
    }
}

bool Telemetry::addData(TelemetryDataRecord *dataRecord) {
    memcpy(&data[index_ofnext++ & mask_for_index], dataRecord, sizeof(TelemetryDataRecord));

}

