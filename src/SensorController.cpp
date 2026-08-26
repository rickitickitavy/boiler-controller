//
// Created by dsporykhin on 08.03.22.
//

#include "SensorController.h"
#include "Converter.h"

SensorController::SensorController(int one_wire_pin, int one_wire_pin_2, SettingsManager *settingsManager) {
#ifdef ENABLE_MODELLING
    this->coreModel = nullptr;
#endif
    this->settingsManager = settingsManager;
    this->last_time_sensors_read = 0;
    memset(sensor_data, 0, sizeof(sensor_data));
    data_ready = false;

    dallasTemperature = new DallasTemperature(new OneWire(one_wire_pin));
//    dallasTemperature->isParasitePowerMode()
    dallasTemperature->begin();

    dallasTemperature2 = one_wire_pin_2 ?
                         new DallasTemperature(new OneWire(one_wire_pin_2))
                                        : nullptr;

    if (dallasTemperature2)
        dallasTemperature2->begin();


    LOGGER.info("   checking channel 0...");
    int sensor_count_0 = countSensors(dallasTemperature);
    found_sensors_count = sensor_count_0;

    LOGGER.info("   checking channel 1...");
    int sensor_count_1 = countSensors(dallasTemperature2);

    found_sensors_count += sensor_count_1;

    if ((!sensor_count_1) && (dallasTemperature2))
        dallasTemperature2 = nullptr;

    if ((sensor_count_1) && (!sensor_count_0))
        dallasTemperature = nullptr;

    char addr_ascii_hex_buffer[SENSORS_ADDR_SIZE * 2 + 1];

    String sens_addr;

    // SMA for all sensors values
    smaSensors = (Sma **) malloc(sizeof(Sma *) * MAX_SENSORS_COUNT);

    int intervals = settingsManager->getSettings()->heaterSettings.temperatureSettings.SMA_temperature_period_sec * 1000
                            / settingsManager->getSettings()->heaterSettings.scan_interval_ms + 1;
    LOGGER.info("   SMA intervals = " + String(intervals));

    for (int index = 0; index < MAX_SENSORS_COUNT; index++)
        smaSensors[index] = new Sma(intervals);

    // reset sensors states
    for (int index = 0; index < MAX_SENSORS_COUNT; index++){
        sensor_data[index].error_state = false;
        sensor_data[index].total_errors_count = 0;
        sensor_data[index].total_success_count = 0;
        sensor_data[index].last_state_count = 0;
    }

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

    cycle_status = STATUS_CYCLE_AWAITING;
}


int SensorController::countSensors(DallasTemperature *dallasTemperature) {
    if (!dallasTemperature){
        LOGGER.info("   channel is absent.");
        return 0;
    }

    dallasTemperature->setWaitForConversion(true);
    int sensors_count = dallasTemperature->getDS18Count();
    char addr_ascii_hex_buffer[SENSORS_ADDR_SIZE * 2 + 1];

    LOGGER.info("   found " + String(sensors_count) + " sensors.");

    for (int termo_index = 0; termo_index < sensors_count; termo_index++) {
        dallasTemperature->getAddress(&found_sensors_addr[(found_sensors_count + termo_index) * SENSORS_ADDR_SIZE], termo_index);
        Converter::bytesToAsciiHex(addr_ascii_hex_buffer, &found_sensors_addr[(found_sensors_count + termo_index) * SENSORS_ADDR_SIZE],
                                   SENSORS_ADDR_SIZE);
        Serial.printf("   temperature sensor %d found (%s)\n", termo_index, addr_ascii_hex_buffer);
    }
    dallasTemperature->setResolution(12);

    return sensors_count;
}



#ifdef ENABLE_MODELLING
void SensorController::setModeller(CoreModel *coreModel) {
    this->coreModel = coreModel;
}
#endif

bool SensorController::hasData(char *data, int size) {
    for (int index = 0; index < size; index++)
        if (data[index])
            return true;

    return false;
}

#ifdef ENABLE_MODELLING
void SensorController::saveModelledSensorValue(int sensor_index, double value) {
    sensor_data[sensor_index].value = value;
    sensor_data[sensor_index].last_time_read = millis();
    sensor_data[sensor_index].data_ready = true;
    smaSensors[sensor_index]->addValue(value);
}
#endif

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

bool SensorController::readDallasSensor(int sensor_index){

    if (hasData(&settingsManager->getSettings()->ds18D20Addresses[sensor_index * SENSORS_ADDR_SIZE],
                SENSORS_ADDR_SIZE)) {
        float tempr = cycle_dallas_temperature->getTempC(
                (uint8_t *) &settingsManager->getSettings()->ds18D20Addresses[sensor_index * SENSORS_ADDR_SIZE]);
        if (tempr != -127) {
            sensor_data[sensor_index].value = tempr;
            sensor_data[sensor_index].last_time_read = last_time_sensors_read;
            sensor_data[sensor_index].data_ready = true;
            smaSensors[sensor_index]->addValue(tempr);
        }

        if (sensor_data[sensor_index].data_ready){
            if (sensor_data[sensor_index].error_state)
                sensor_data[sensor_index].last_state_count = 1;
            else
                sensor_data[sensor_index].last_state_count++;

            sensor_data[sensor_index].error_state = false;
            sensor_data[sensor_index].total_success_count++;
        } else {
            if (!sensor_data[sensor_index].error_state)
                sensor_data[sensor_index].last_state_count = 1;
            else
                sensor_data[sensor_index].last_state_count++;

            sensor_data[sensor_index].error_state = true;
            sensor_data[sensor_index].total_errors_count++;
        }
        return true;
    } else
        return false;
}

void SensorController::startAsyncConversion(){

    // init async conversion
    cycle_dallas_temperature->setWaitForConversion(false);
    cycle_dallas_temperature->requestTemperatures();
    cycle_conversion_started_at = millis();

}

void SensorController::readNextBlockOfSensors(){
    if (cycle_status == STATUS_CYCLE_AWAITING){

        cycle_status = STATUS_CYCLE_STARTED;

        for (auto & index : sensor_data)
            index.data_ready = false;

        // select first block
        if (dallasTemperature != nullptr)
            cycle_current_dallas_block = 0;
        else if (dallasTemperature2 != nullptr)
            cycle_current_dallas_block = 1;
        else {
            cycle_status = STATUS_CYCLE_DONE;
            return;
        }

        // select sensorsDriver
        cycle_dallas_temperature = cycle_current_dallas_block == 0
                ? dallasTemperature
                : dallasTemperature2;

        startAsyncConversion();

    }

    if (cycle_status == STATUS_CYCLE_STARTED){

        bool go_to_next_block = false;

        if (cycle_dallas_temperature->isConversionComplete()){

            cycle_dallas_temperature->blockTillConversionComplete(cycle_dallas_temperature->getResolution());

            for (int i =0; i < MAX_SENSORS_COUNT; readDallasSensor(i++));

            go_to_next_block = true;

        } else if ((millis() - cycle_conversion_started_at) > 1000){
            // timed out
            Serial.printf("   DS18B20: TIMED OUT FOR: %d.\n", cycle_current_dallas_block);
            go_to_next_block = true;
        }

        if (go_to_next_block) {
            if ((cycle_current_dallas_block == 0) && dallasTemperature2) {
                // the next block of dallas sensor is present;
                cycle_current_dallas_block = 1;
                cycle_dallas_temperature = dallasTemperature2;
                startAsyncConversion();
            } else {
                // there is no more sensors. The cycle is done
                cycle_status = STATUS_CYCLE_DONE;
            }
        }
    }
}

void SensorController::fire() {
#ifdef ENABLE_MODELLING
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
    else
#endif
    {
        // data ALREADY ready. see
        if (cycle_status == STATUS_CYCLE_DONE) {
            cycle_status = STATUS_CYCLE_AWAITING;
        }

    }

    data_ready = true;
}

float SensorController::getSmaValue(int sensorIndex) {
    int intervals = settingsManager->getSettings()->heaterSettings.temperatureSettings.SMA_temperature_period_sec * 1000
                    / settingsManager->getSettings()->heaterSettings.scan_interval_ms + 1;
    return smaSensors[sensorIndex]->calcSma(intervals);
}

float SensorController::getNotNANSmaValue(int sensorIndex) {
    float val = getSmaValue(sensorIndex);
    return isnan(val) ? 0 : val;
}

float SensorController::getNotNANValue(int sensorIndex) {
    float val = sensor_data[sensorIndex].value;
    return isnan(val) ? 0 : val;
}