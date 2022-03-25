//
// Created by dsporykhin on 24.04.20.
//

#include <IPAddress.h>
#include "SettingsNavigator.h"
#include "Logger.h"
#include "Converter.h"

SettingsNavigator::SettingsNavigator(SettingsManager *settingsManager) {
    this->settingsManager = settingsManager;
    this->settings = settingsManager->getSettings();

//    this->paramDescriptors[activeParamDescriptors++] = new ParamDescriptor("flame>shutdownAfterSec", INTEGER, 5, 78600,
//                                                                           (void *) &settings->activeProfile.flameSettings.shutdownAfterSec,
//                                                                           (void *) &tempProfile.flameSettings.shutdownAfterSec);
//    this->paramDescriptors[activeParamDescriptors++] = new ParamDescriptor("flame>brightnessCoefficientPrct", FLOAT, 0,
//                                                                           100,
//                                                                           (void *) &settings->activeProfile.flameSettings.brightnessCoefficientPrct,
//                                                                           (void *) &tempProfile.flameSettings.brightnessCoefficientPrct);

//    this->paramDescriptors[activeParamDescriptors++] = new ParamDescriptor("network>wifiMode", UCHAR, 0,
//                                                                           10,
//                                                                           (void *) &settings->network.wifiMode,
//                                                                           (void *) &settings->network.wifiMode);
    this->paramDescriptors[activeParamDescriptors++] = new ParamDescriptor("network>ssid", STRING, 5,
                                                                           63,
                                                                           (void *) &settings->network.ssid[0],
                                                                           (void *) &settings->network.ssid[0]);
    this->paramDescriptors[activeParamDescriptors++] = new ParamDescriptor("network>password", STRING, 5,
                                                                           63,
                                                                           (void *) &settings->network.password[0],
                                                                           (void *) &settings->network.password[0]);

   this->paramDescriptors[activeParamDescriptors++] = new ParamDescriptor("mqtt>server", STRING, 5,
                                                                           63,
                                                                           (void *) &settings->mqttServer[0],
                                                                           (void *) &settings->mqttServer[0]);
    this->paramDescriptors[activeParamDescriptors++] = new ParamDescriptor("mqtt>port", INTEGER, 30, 65534,
                                                                           (void *) &settings->mqttPort,
                                                                           (void *) &settings->mqttPort);
    this->paramDescriptors[activeParamDescriptors++] = new ParamDescriptor("mqtt>reconnectIntervalMs", INTEGER, 30, 60000,
                                                                           (void *) &settings->mqttReconnectIntervalMs,
                                                                           (void *) &settings->mqttReconnectIntervalMs);
    this->paramDescriptors[activeParamDescriptors++] = new ParamDescriptor("mqtt>deviceName", STRING, 2,
                                                                           31,
                                                                           (void *) &settings->mqttDeviceName[0],
                                                                           (void *) &settings->mqttDeviceName[0]);
    this->paramDescriptors[activeParamDescriptors++] = new ParamDescriptor("mqtt>serverBornTopic", STRING, 2,
                                                                           63,
                                                                           (void *) &settings->mqttServerBornTopic[0],
                                                                           (void *) &settings->mqttServerBornTopic[0]);
    this->paramDescriptors[activeParamDescriptors++] = new ParamDescriptor("device>stateOutgoingTopicPrefix", STRING, 5,
                                                                           31,
                                                                           (void *) &settings->deviceStateOutgoingTopicPrefix[0],
                                                                           (void *) &settings->deviceStateOutgoingTopicPrefix[0]);
    this->paramDescriptors[activeParamDescriptors++] = new ParamDescriptor("device>iHaveBornTopic", STRING, 5,
                                                                           63,
                                                                           (void *) &settings->deviceIHaveBornTopic[0],
                                                                           (void *) &settings->deviceIHaveBornTopic[0]);
    this->paramDescriptors[activeParamDescriptors++] = new ParamDescriptor("device>incomingCommandTopicPrefix", STRING, 5,
                                                                           31,
                                                                           (void *) &settings->deviceIncomingCommandTopicPrefix[0],
                                                                           (void *) &settings->deviceIncomingCommandTopicPrefix[0]);

    this->paramDescriptors[activeParamDescriptors++] = new ParamDescriptor("device>mqttInputToolTopic", STRING, 5,
                                                                           31,
                                                                           (void *) &settings->mqttInputToolTopic[0],
                                                                           (void *) &settings->mqttInputToolTopic[0]);
    this->paramDescriptors[activeParamDescriptors++] = new ParamDescriptor("device>mqttOutputToolTopic", STRING, 5,
                                                                           31,
                                                                           (void *) &settings->mqttOutputToolTopic[0],
                                                                           (void *) &settings->mqttOutputToolTopic[0]);
    this->paramDescriptors[activeParamDescriptors++] = new ParamDescriptor("device>defaultSwitcherState", UCHAR, 0,
                                                                           255,
                                                                           (void *) &settings->defaultSwitcherState,
                                                                           (void *) &settings->defaultSwitcherState);
//    this->paramDescriptors[activeParamDescriptors++] = new ParamDescriptor("device>statefull", UCHAR, 0,
//                                                                           255,
//                                                                           (void *) &settings->statefull,
//                                                                           (void *) &settings->statefull);
    this->paramDescriptors[activeParamDescriptors++] = new ParamDescriptor("device>dimmerPwmValue", INTEGER, 0,
                                                                           1024,
                                                                           (void *) &settings->dimmerPwmValue,
                                                                           (void *) &settings->dimmerPwmValue);
//    this->paramDescriptors[activeParamDescriptors++] = new ParamDescriptor("device>dimmerPwmValue", INTEGER, 0,
//                                                                           1024,
//                                                                           (void *) &settings->dimmerPwmValue,
//                                                                           (void *) &settings->dimmerPwmValue);
//    this->paramDescriptors[activeParamDescriptors++] = new ParamDescriptor("network>ipAddress", IPv4, 7,
//                                                                           15,
//                                                                           (void *) &settings->network.ipAddress[0],
//                                                                           (void *) &settings->network.ipAddress[0]);
    this->paramDescriptors[activeParamDescriptors++] = new ParamDescriptor("sensors>heater>core", SENSORS_ADDR_SIZE,
                                                                           (void *) &settings->ds18D20Addresses[0],
                                                                           (void *) &settings->ds18D20Addresses[0]);
    this->paramDescriptors[activeParamDescriptors++] = new ParamDescriptor("sensors>heater>output_flow", SENSORS_ADDR_SIZE,
                                                                           (void *) &settings->ds18D20Addresses[SENSORS_ADDR_SIZE * 1],
                                                                           (void *) &settings->ds18D20Addresses[SENSORS_ADDR_SIZE * 1]);
    this->paramDescriptors[activeParamDescriptors++] = new ParamDescriptor("sensors>heater>input_flow", SENSORS_ADDR_SIZE,
                                                                           (void *) &settings->ds18D20Addresses[SENSORS_ADDR_SIZE * 2],
                                                                           (void *) &settings->ds18D20Addresses[SENSORS_ADDR_SIZE * 2]);

    this->paramDescriptors[activeParamDescriptors++] = new ParamDescriptor("sensors>termoaccumulator>top", SENSORS_ADDR_SIZE,
                                                                           (void *) &settings->ds18D20Addresses[SENSORS_ADDR_SIZE * 3],
                                                                           (void *) &settings->ds18D20Addresses[SENSORS_ADDR_SIZE * 3]);
    this->paramDescriptors[activeParamDescriptors++] = new ParamDescriptor("sensors>termoaccumulator>middle_hi", SENSORS_ADDR_SIZE,
                                                                           (void *) &settings->ds18D20Addresses[SENSORS_ADDR_SIZE * 4],
                                                                           (void *) &settings->ds18D20Addresses[SENSORS_ADDR_SIZE * 4]);
    this->paramDescriptors[activeParamDescriptors++] = new ParamDescriptor("sensors>termoaccumulator>middle_low", SENSORS_ADDR_SIZE,
                                                                           (void *) &settings->ds18D20Addresses[SENSORS_ADDR_SIZE * 5],
                                                                           (void *) &settings->ds18D20Addresses[SENSORS_ADDR_SIZE * 5]);
    this->paramDescriptors[activeParamDescriptors++] = new ParamDescriptor("sensors>termoaccumulator>bottom", SENSORS_ADDR_SIZE,
                                                                           (void *) &settings->ds18D20Addresses[SENSORS_ADDR_SIZE * 6],
                                                                           (void *) &settings->ds18D20Addresses[SENSORS_ADDR_SIZE * 6]);
    this->paramDescriptors[activeParamDescriptors++] = new ParamDescriptor("sensors>forward>temperature", SENSORS_ADDR_SIZE,
                                                                           (void *) &settings->ds18D20Addresses[SENSORS_ADDR_SIZE * 7],
                                                                           (void *) &settings->ds18D20Addresses[SENSORS_ADDR_SIZE * 7]);
    this->paramDescriptors[activeParamDescriptors++] = new ParamDescriptor("sensors>back>contour1", SENSORS_ADDR_SIZE,
                                                                           (void *) &settings->ds18D20Addresses[SENSORS_ADDR_SIZE * 8],
                                                                           (void *) &settings->ds18D20Addresses[SENSORS_ADDR_SIZE * 8]);
    this->paramDescriptors[activeParamDescriptors++] = new ParamDescriptor("sensors>back>contour2", SENSORS_ADDR_SIZE,
                                                                           (void *) &settings->ds18D20Addresses[SENSORS_ADDR_SIZE * 9],
                                                                           (void *) &settings->ds18D20Addresses[SENSORS_ADDR_SIZE * 9]);
    this->paramDescriptors[activeParamDescriptors++] = new ParamDescriptor("sensors>back>contour3", SENSORS_ADDR_SIZE,
                                                                           (void *) &settings->ds18D20Addresses[SENSORS_ADDR_SIZE * 10],
                                                                           (void *) &settings->ds18D20Addresses[SENSORS_ADDR_SIZE * 10]);
    this->paramDescriptors[activeParamDescriptors++] = new ParamDescriptor("sensors>scan_integrval_ms", INTEGER, 3000,
                                                                           120000,
                                                                           (void *) &settings->scan_sensors_integrval_ms,
                                                                           (void *) &settings->scan_sensors_integrval_ms);
    this->paramDescriptors[activeParamDescriptors++] = new ParamDescriptor("control>servo>pwm_controller_address", UCHAR, 1,
                                                                           127,
                                                                           (void *) &settings->pwm_controller_address,
                                                                           (void *) &settings->pwm_controller_address);
    this->paramDescriptors[activeParamDescriptors++] = new ParamDescriptor("control>servo>smoke_pipe_control_channel_id", UCHAR, 0,
                                                                           15,
                                                                           (void *) &settings->smoke_pipe_control_channel_id,
                                                                           (void *) &settings->smoke_pipe_control_channel_id);
    this->paramDescriptors[activeParamDescriptors++] = new ParamDescriptor("control>servo>oxygen_door_control_channel_id", UCHAR, 0,
                                                                           15,
                                                                           (void *) &settings->oxygen_door_control_channel_id,
                                                                           (void *) &settings->oxygen_door_control_channel_id);
    this->paramDescriptors[activeParamDescriptors++] = new ParamDescriptor("control>servo>upper_door_control_channel_id", UCHAR, 0,
                                                                           15,
                                                                           (void *) &settings->upper_door_control_channel_id,
                                                                           (void *) &settings->upper_door_control_channel_id);
//
}
//--------------------------------------------------------------------
void SettingsNavigator::addParamDescriptor(ParamDescriptor *descriptor){
    this->paramDescriptors[activeParamDescriptors++] = descriptor;
}

//--------------------------------------------------------------------

String SettingsNavigator::getSettingByName(String origParamName) {
    String paramName = origParamName;

    bool showMinR = paramName.endsWith("#minR");
    bool showMin = paramName.endsWith("#min");
    bool showMaxR = paramName.endsWith("#maxR");
    bool showMax = paramName.endsWith("#max");

    // проверим на наличие #min или #max в конце имени переменной
    if (showMax || showMin) {
        paramName.remove(origParamName.length() - 4);
    }  else if (showMaxR || showMinR) {
        paramName.remove(origParamName.length() - 5);
    }

    for (int descriptorIndex = 0; descriptorIndex < activeParamDescriptors; descriptorIndex++) {
        if (paramDescriptors[descriptorIndex]->paramName == paramName) {
            // Нашли наш параметер
            if (paramDescriptors[descriptorIndex]->arraySize == 0 || showMax || showMin || showMaxR || showMinR) {
                if (paramDescriptors[descriptorIndex]->paramType == INTEGER || showMaxR || showMinR) {
                    return showMin || showMax || showMinR || showMaxR
                           ? (showMin || showMinR ? String((int) paramDescriptors[descriptorIndex]->minValue) : String(
                                    (int) paramDescriptors[descriptorIndex]->maxValue))
                           : String(*(int *) paramDescriptors[descriptorIndex]->valueReferenceForRead);
                } else if (paramDescriptors[descriptorIndex]->paramType == BOOLEAN) {
//                    LOGGER.info("PTR = " +  String((int)(paramDescriptors[descriptorIndex]->valueReferenceForRead)));
//                    uint8_t _data = *(bool*)paramDescriptors[descriptorIndex]->valueReferenceForRead;
//                    LOGGER.info("DATA = " +  String(_data));

                    return *(bool *) paramDescriptors[descriptorIndex]->valueReferenceForRead ? "true" : "false";
                }else if (paramDescriptors[descriptorIndex]->paramType == FLOAT) {
                    return showMin || showMax
                           ? (showMin ? String(paramDescriptors[descriptorIndex]->minValue) : String(
                                    paramDescriptors[descriptorIndex]->maxValue))
                           : String(*(float *) paramDescriptors[descriptorIndex]->valueReferenceForRead);
                } else if (paramDescriptors[descriptorIndex]->paramType == STRING){
                    return showMin || showMax
                           ? (showMin ? String((int)paramDescriptors[descriptorIndex]->minValue) : String(
                                    (int)paramDescriptors[descriptorIndex]->maxValue))
                           : String((char *)paramDescriptors[descriptorIndex]->valueReferenceForRead);
                } else if (paramDescriptors[descriptorIndex]->paramType == UCHAR){
                    return showMin || showMax
                           ? (showMin ? String((unsigned char)paramDescriptors[descriptorIndex]->minValue) : String(
                                    (unsigned char)paramDescriptors[descriptorIndex]->maxValue))
                           : String(*(unsigned char *)paramDescriptors[descriptorIndex]->valueReferenceForRead);
                } else if (paramDescriptors[descriptorIndex]->paramType == IPv4){
                    String ipAsString;

                    if (!showMin && !showMax){
                        unsigned char *ipRaw = (unsigned char *)paramDescriptors[descriptorIndex]->valueReferenceForRead;
                        IPAddress ipV4 = IPAddress(ipRaw[0], ipRaw[1], ipRaw[2], ipRaw[3]);
                        ipAsString = ipV4.toString();
                    }
                    return showMin || showMax
                           ? (showMin ? String((unsigned char)paramDescriptors[descriptorIndex]->minValue) : String(
                                    (unsigned char)paramDescriptors[descriptorIndex]->maxValue))
                           : ipAsString;
                } else if (paramDescriptors[descriptorIndex]->paramType == HEX_BYTES) {
                    // the maximum length of hex string we will made
                    char buffer[257];
                    Converter::bytesToAsciiHex(buffer, (uint8_t*)paramDescriptors[descriptorIndex]->valueReferenceForRead, paramDescriptors[descriptorIndex]->byte_data_length);
                    return String(buffer);
                }
            }  else {
                //  array
                String paramNameTitle = paramName;
                paramNameTitle.replace('>', '_');
                String result = "{\"" + paramNameTitle + "\":[";

                if (paramDescriptors[descriptorIndex]->paramType == INTEGER) {
                    int *arrayRef = (int *) paramDescriptors[descriptorIndex]->valueReferenceForRead;

                    for (int i = 0; i < paramDescriptors[descriptorIndex]->arraySize; i++) {
                        if (i != 0) {
                            result.concat(",");
                        }
                        result.concat(String(arrayRef[i]));
                    }
                } else if (paramDescriptors[descriptorIndex]->paramType == FLOAT) {
                    float *arrayRef = (float *) paramDescriptors[descriptorIndex]->valueReferenceForRead;

                    for (int i = 0; i < paramDescriptors[descriptorIndex]->arraySize; i++) {
                        if (i != 0) {
                            result.concat(",");
                        }
                        result.concat(String(arrayRef[i]));
                    }
                }
                result.concat("]}");
                return result;
            }
        }
    }

    LOGGER.error("parameter " + paramName + " not found");
    return "bad parameter name";

}
//--------------------------------------------------------------------


void SettingsNavigator::saveNetworkSettingsAndRestart(NetworkSettings *networkSettings) {
    LOGGER.warning("Saving new network settings:\r\n"
                           "        SSID: " + String(networkSettings->ssid) + "\r\n"
                           "    password: " + String(networkSettings->password));

    memcpy(&settings->network, networkSettings, sizeof(NetworkSettings));

    settingsManager->saveSetting(true);
}
//--------------------------------------------------------------------


String SettingsNavigator::saveSettingsByNames(String *params, int paramsCount) {
    String res = "";

    for (int paramIndex = 0; paramIndex < paramsCount; paramIndex++) {
        String param = params[paramIndex];
        param.trim();

        if (!param.isEmpty()) {
// отделим значение от имени параметра
            String invalidParameterMessage = ("Invalid parameter format (\"" + param + "\"");
            int separatorIndex = param.indexOf('=');
            if (separatorIndex == -1) {
                return invalidParameterMessage;
            }

            String paramName = param.substring(0, separatorIndex);
            String value = param.substring(separatorIndex + 1);

            paramName.trim();
            value.trim();

            if (param.isEmpty() || value.isEmpty()) {
                return invalidParameterMessage;
            }

            char tempBuf[64];
            res = saveSettingByName(paramName, value);
        }
        // Если ошибка записи переменной, то далее не продолжаем
        if (res != "") {
            return res;
        }
    }

    settingsManager->saveSetting(false);

    settingsManager->logSettings();

    return res;
}
//--------------------------------------------------------------------

void SettingsNavigator::setSensorList(String sensorsList){
    this->sensorsList = (char*)malloc(sensorsList.length() + 1);
    memcpy(this->sensorsList, sensorsList.c_str(), sensorsList.length());
    this->sensorsList[sensorsList.length()] = 0;
    this->paramDescriptors[activeParamDescriptors++] = new ParamDescriptor("sensors>list", STRING, 0,
                                                                           0,
                                                                           (void *) this->sensorsList,
                                                                           (void *) this->sensorsList);
}
//--------------------------------------------------------------------

String SettingsNavigator::saveSettingByName(String paramName, String value) {
    for (int descriptorIndex = 0; descriptorIndex < activeParamDescriptors; descriptorIndex++) {
        if (paramDescriptors[descriptorIndex]->paramName == paramName) {
            // Нашли наш параметер
            if (paramDescriptors[descriptorIndex]->arraySize == 0) {
                if (paramDescriptors[descriptorIndex]->paramType == INTEGER) {
                    int intValue = value.toInt();
                    if ((intValue < paramDescriptors[descriptorIndex]->minValue)
                        || (intValue > paramDescriptors[descriptorIndex]->maxValue)) {
                        return "Value of \"" + paramName + "\" is not in diapason from "
                               + String(paramDescriptors[descriptorIndex]->minValue) + " to "
                               + String(paramDescriptors[descriptorIndex]->maxValue);
                    } else {
                        *((int *) paramDescriptors[descriptorIndex]->valueReferenceForWrite) = intValue;
                    }
                } else if (paramDescriptors[descriptorIndex]->paramType == UCHAR) {
                    uint8_t ucharValue = value.toInt();
                    if ((ucharValue < paramDescriptors[descriptorIndex]->minValue)
                        || (ucharValue > paramDescriptors[descriptorIndex]->maxValue)) {
                        return "Value of \"" + paramName + "\" is not in diapason from "
                               + String(paramDescriptors[descriptorIndex]->minValue) + " to "
                               + String(paramDescriptors[descriptorIndex]->maxValue);
                    } else {
                        *((uint8_t *) paramDescriptors[descriptorIndex]->valueReferenceForWrite) = ucharValue;
                    }
                } else if (paramDescriptors[descriptorIndex]->paramType == FLOAT) {
                    float floatValue = value.toFloat();
                    if ((floatValue < paramDescriptors[descriptorIndex]->minValue)
                        || (floatValue > paramDescriptors[descriptorIndex]->maxValue)) {
                        return "Value of \"" + paramName + "\" is not in diapason from "
                               + String(paramDescriptors[descriptorIndex]->minValue) + " to "
                               + String(paramDescriptors[descriptorIndex]->maxValue);
                    } else {
                        *((float *) paramDescriptors[descriptorIndex]->valueReferenceForWrite) = floatValue;
                    }
                } else if (paramDescriptors[descriptorIndex]->paramType == STRING){
                    int valLen = value.length();
                    if ((valLen < paramDescriptors[descriptorIndex]->minValue)
                            || (valLen > paramDescriptors[descriptorIndex]->maxValue)){
                        return "Length of \"" + paramName + "\" is not in diapason from "
                               + String(paramDescriptors[descriptorIndex]->minValue) + " to "
                               + String(paramDescriptors[descriptorIndex]->maxValue);
                    } else {
                        memset(paramDescriptors[descriptorIndex]->valueReferenceForWrite, 0, valLen+1);
                        memcpy(paramDescriptors[descriptorIndex]->valueReferenceForWrite, value.c_str(), valLen);
                    }
                } else if (paramDescriptors[descriptorIndex]->paramType == HEX_BYTES) {
                    // the maximum length of hex string we will made
                    if ((value.length() >> 1) != paramDescriptors[descriptorIndex]->byte_data_length)
                        return "Length of \"" + paramName + "\" is " + String(value.length())
                               + " but must be " + String(paramDescriptors[descriptorIndex]->byte_data_length);

                    uint8_t buffer[128];
                    if (!Converter::asciiHexToBytes(&buffer[0], value.c_str(),
                                                    paramDescriptors[descriptorIndex]->byte_data_length))
                        return "Failed to convert " + value + " to bytes";

                    memcpy(paramDescriptors[descriptorIndex]->valueReferenceForWrite, buffer,
                           paramDescriptors[descriptorIndex]->byte_data_length);
                } else {
                    return "Unsupported data format " + String(paramDescriptors[descriptorIndex]->paramType);
                }
            } else {
                // массив
                if (paramDescriptors[descriptorIndex]->paramType == FLOAT) {

                    char elementSeparator = ',';

                    int indexOfParamSeparator = value.indexOf(elementSeparator);
                    int startCopyIndex = 0;
                    int matrixIndex = 0;
                    String strValue;
                    if (value.indexOf(elementSeparator) >= 0)
                        do {
                            strValue = indexOfParamSeparator == -1
                                       ? value.substring(startCopyIndex)
                                       : value.substring(startCopyIndex, indexOfParamSeparator);
                            startCopyIndex = indexOfParamSeparator + 1;

                            float *refMatrix = (float *) paramDescriptors[descriptorIndex]->valueReferenceForWrite;
                            float floatValue = strValue.toFloat();

                            if ((floatValue < paramDescriptors[descriptorIndex]->minValue)
                                || (floatValue > paramDescriptors[descriptorIndex]->maxValue)) {
                                return "Value of \"" + paramName + "\" is not in diapason from "
                                       + String(paramDescriptors[descriptorIndex]->minValue) + " to "
                                       + String(paramDescriptors[descriptorIndex]->maxValue);
                            }
                            refMatrix[matrixIndex] = floatValue;
                            matrixIndex++;
                        } while ((indexOfParamSeparator = value.indexOf(elementSeparator, indexOfParamSeparator + 1)) >=
                                 0 &&
                                 matrixIndex < 32);
                    // последний элемент
                    if (startCopyIndex < value.length() && matrixIndex < 32) {
                        strValue = indexOfParamSeparator == -1
                                   ? value.substring(startCopyIndex)
                                   : value.substring(startCopyIndex, indexOfParamSeparator);

                        float *refMatrix = (float *) paramDescriptors[descriptorIndex]->valueReferenceForWrite;
                        float floatValue = strValue.toFloat();

                        if ((floatValue < paramDescriptors[descriptorIndex]->minValue)
                            || (floatValue > paramDescriptors[descriptorIndex]->maxValue)) {
                            return "Value of \"" + paramName + "\" is not in diapason from "
                                   + String(paramDescriptors[descriptorIndex]->minValue) + " to "
                                   + String(paramDescriptors[descriptorIndex]->maxValue);
                        }
                        refMatrix[matrixIndex] = floatValue;
                    }
                }
            }

            return "";
        }
    }
    return "parameter \"" + paramName + "\" not found";
}
//--------------------------------------------------------------------

