//
// Created by dsporykhin on 08.03.22.
//

#include "SensorController.h"
#include "Converter.h"

SensorController::SensorController(int one_wire_pin, SettingsManager *settingsManager) {
    this->coreModel = nullptr;
    this->settingsManager = settingsManager;
    this->last_time_sensors_read = 0;
    memset(sensor_data, 0, sizeof(sensor_data));
    data_ready = false;

    oneWire = new OneWire(one_wire_pin);
    dallasTemperature = new DallasTemperature(oneWire);

//    dallasTemperature->isParasitePowerMode()

    dallasTemperature->begin();
    found_sensors_count = dallasTemperature->getDS18Count();

    char addr_ascii_hex_buffer[SENSORS_ADDR_SIZE * 2 + 1];
    String sens_addr;

    for (int termo_index = 0; termo_index < found_sensors_count; termo_index++) {
        dallasTemperature->getAddress(&found_sensors_addr[termo_index * SENSORS_ADDR_SIZE], termo_index);
        Converter::bytesToAsciiHex(addr_ascii_hex_buffer, &found_sensors_addr[termo_index * SENSORS_ADDR_SIZE],
                                   SENSORS_ADDR_SIZE);
        sens_addr = String(addr_ascii_hex_buffer);
        LOGGER.info("   temperature sensor " + String(termo_index) + " found (" + sens_addr + ")");
    }
    dallasTemperature->setResolution(12);

    // SMA for all sensors values
    smaSensors = (Sma **) malloc(sizeof(Sma *) * MAX_SENSORS_COUNT);
    int intervals = settingsManager->getSettings()->heaterSettings.temperatureSettings.SMA_temperature_period_sec * 1000
                    / settingsManager->getSettings()->heaterSettings.scan_interval_ms + 1;
    for (int index = 0; index < MAX_SENSORS_COUNT; index++)
        smaSensors[index] = new Sma(intervals);

    if (!found_sensors_count) {
        LOGGER.info("   temperature sensor NOT found");
        hasSensors = false;
    } else {
        // sensors found. check if they configured or no
        hasSensors = true;

        bool sensorsConfigured = false;
        for (int search_index = 0; search_index < MAX_SENSORS_COUNT; search_index++)
            if (hasData(&settingsManager->getSettings()->ds18D20Addresses[search_index * SENSORS_ADDR_SIZE],
                        SENSORS_ADDR_SIZE)) {
                Converter::bytesToAsciiHex(
                        addr_ascii_hex_buffer,
                        (uint8_t *) &settingsManager->getSettings()->ds18D20Addresses[search_index * SENSORS_ADDR_SIZE],
                        SENSORS_ADDR_SIZE);
                sens_addr = String(addr_ascii_hex_buffer);
                LOGGER.info("   temperature channel " + String(search_index) + " linked to sensor " + sens_addr);
                sensorsConfigured = true;
            }

        if (!sensorsConfigured) {
            // sensors not configured - then autoconfigure all sensors
            LOGGER.info("Sensors was not configured. Saving all found sensors to config");
            // there is no configured sensors. Write all address of sensors to config
            for (int index = 0; index < found_sensors_count; index++) {
                memcpy(&settingsManager->getSettings()->ds18D20Addresses[index * SENSORS_ADDR_SIZE],
                       &found_sensors_addr[index * SENSORS_ADDR_SIZE], SENSORS_ADDR_SIZE);
            }
            settingsManager->saveSetting(false);
        } else
            LOGGER.info("Sensors was configured fully or partial. Using present config.");
    }
}

void SensorController::setModeller(CoreModel *coreModel) {
    this->coreModel = coreModel;
}

bool SensorController::hasData(char *data, int size) {
    for (int index = 0; index < size; index++)
        if (data[index])
            return true;

    return false;
}

void SensorController::saveModelledSensorValue(int sensor_index, double value) {
    sensor_data[sensor_index].value = value;
    sensor_data[sensor_index].last_time_read = millis();
    sensor_data[sensor_index].data_ready = true;
    smaSensors[sensor_index]->addValue(value);
}

String SensorController::buildSensorsList() {
    String const option_tag_start = "<option value=\"";
    String const option_tag_middle = "\">";
    String const option_tag_end = "</option>\r\n";
    String const option_no_sensor_value = "0000000000000000";
    String result =
            option_tag_start + option_no_sensor_value + option_tag_middle + option_no_sensor_value + option_tag_end;
    char buffer[SENSORS_ADDR_SIZE * 2 + 1];
    for (int index = 0; index < found_sensors_count; index++) {
        Converter::bytesToAsciiHex(buffer, &found_sensors_addr[index * SENSORS_ADDR_SIZE], SENSORS_ADDR_SIZE);
        String address = String(buffer);
        result += option_tag_start + address + option_tag_middle + address + option_tag_end;
    }
    LOGGER.info(result);
    return result;
}

bool SensorController::isHasSensors() {
    return hasSensors;
}

void SensorController::handle() {
    if (hasSensors && (last_time_sensors_read == 0 || ((millis() - last_time_sensors_read) >
                                                       settingsManager->getSettings()->send_data_to_mqtt_interval_ms))) {
        last_time_sensors_read = millis();
        fire();
    }

}

void SensorController::fire() {
    if (coreModel) {
        coreModel->handle();
        saveModelledSensorValue(T_SENS_INDEX_CORE, coreModel->core_tempr);
        saveModelledSensorValue(T_SENS_INDEX_INPUT_FLOW, coreModel->bottom_tempr);
        saveModelledSensorValue(T_SENS_INDEX_ACC_BOTTOM, coreModel->bottom_tempr);
        saveModelledSensorValue(T_SENS_INDEX_OUTPUT_FLOW, coreModel->output_tempr);
        saveModelledSensorValue(T_SENS_INDEX_ACC_TOP, coreModel->top_tempr);
        saveModelledSensorValue(T_SENS_INDEX_ACC_MID_LO, coreModel->lower_tempr);
        saveModelledSensorValue(T_SENS_INDEX_ACC_MID_HI, coreModel->higher_tempr);
        saveModelledSensorValue(T_SENS_INDEX_FORWARD_FLOW, coreModel->forward_tempr);
        saveModelledSensorValue(T_SENS_INDEX_BACKWARD_FLOW, coreModel->backward_tempr);
    }
    else {
        dallasTemperature->requestTemperatures();
        for (int index = 0; index < MAX_SENSORS_COUNT; index++) {
            sensor_data[index].data_ready = false;
            if (hasData(&settingsManager->getSettings()->ds18D20Addresses[index * SENSORS_ADDR_SIZE],
                        SENSORS_ADDR_SIZE)) {
                float tempr = dallasTemperature->getTempC(
                        (uint8_t *) &settingsManager->getSettings()->ds18D20Addresses[index * SENSORS_ADDR_SIZE]);
                if (tempr != -127) {
                    sensor_data[index].value = tempr;
                    sensor_data[index].last_time_read = last_time_sensors_read;
                    sensor_data[index].data_ready = true;
                    smaSensors[index]->addValue(tempr);
                }
            }
        }
    }

    data_ready = true;
}

double SensorController::getSmaValue(int sensorIndex) {
    int intervals = settingsManager->getSettings()->heaterSettings.temperatureSettings.SMA_temperature_period_sec * 1000
                    / settingsManager->getSettings()->heaterSettings.scan_interval_ms + 1;
    return smaSensors[sensorIndex]->calcSma(intervals);
}

double SensorController::getNotNANSmaValue(int sensorIndex) {
    double val = getSmaValue(sensorIndex);
    return isnan(val) ? 0 : val;
}

double SensorController::getNotNANValue(int sensorIndex) {
    double val = sensor_data[sensorIndex].value;
    return isnan(val) ? 0 : val;
}