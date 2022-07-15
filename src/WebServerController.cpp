//
// Created by dsporykhin on 19.04.20.
//

#include <SD.h>
#include "WebServerController.h"
#include "SPIFFS.h"
#include "Defines.h"
#include "Logger.h"

String TEXT_PLAN = "text/plan";
String TEXT_JSON = "text/json";
String OK_RESPONSE = "OK";
String PROFILES_PARAMETER_ATTR_NAME = "parameter";

SettingsManager *WebServerController::settingsManager;
HeaterController *WebServerController::heaterController;

WebServerController::WebServerController(SettingsManager *settingsManager) {
    WebServerController::settingsManager = settingsManager;
    this->heaterController = nullptr;

    LOGGER.info(" Starting web server...");

    webServer = new AsyncWebServer(80);

    webServer->on("/", HTTP_GET, [](AsyncWebServerRequest *request) {
        request->send(SPIFFS, "/index.html", String(), false, systemSettingsProcessor);
    });

    webServer->on("/index.html", HTTP_GET, [](AsyncWebServerRequest *request) {
        request->send(SPIFFS, "/index.html", String(), false, systemSettingsProcessor);
    });

    webServer->on("/settings.html", HTTP_GET, [](AsyncWebServerRequest *request) {
        request->send(SPIFFS, "/settings.html", String(), false, systemSettingsProcessor);
    });

//    webServer->on("/log", HTTP_GET, [](AsyncWebServerRequest *request) {
//        request->send(200, "text/html", &LOGGER.logData[0]);
//    });
//
    webServer->on("/settingsApi", HTTP_GET, settingsApiProcessor);
    webServer->on("/settingsApi", HTTP_POST, settingsApiProcessor);
    webServer->on("/startModelling", HTTP_GET, startModelling);
    webServer->on("/stopModelling", HTTP_GET, stopModelling);
    webServer->on("/getTelemetry", HTTP_GET, getTelemetry);

    webServer->onNotFound(loadFileByUrl);

    webServer->begin();
}

/**
 * Change system settings handler
 * @param request
 */
void WebServerController::settingsApiProcessor(AsyncWebServerRequest *request) {

    LOGGER.info("apiSettings");

    String parameterAttrName = "parameter";
    String valueAttrName = "value";

    String operation = request->arg(PROFILES_OPERATION_ATTR_NAME);
    String parameter = request->arg(parameterAttrName);

    if (!parameter || parameter.isEmpty()) {
        String message = "\"" + parameterAttrName + "\" can't be null";
        request->send(200, TEXT_PLAN, message);
    } else if (parameter == "wifi") {
        if (operation = PARAMETER_OPERATION_WRITE) {
            LOGGER.info("all setting.");

            String wifiSSID = request->arg("network>ssid");
            String wifiPassword = request->arg("network>password");
            String mqttServer = request->arg("mqtt>server");
            String mqttPort = request->arg("mqtt>port");
            String mqttDeviceName = request->arg("mqtt>deviceName");
            String mqttReconnectIntervalMs = request->arg("mqtt>reconnectIntervalMs");

            LOGGER.info("   start check");

            if (!wifiSSID || wifiSSID.isEmpty()) {
                request->send(503, TEXT_PLAN, "wiFiSsId can't be null");
            } else if (!wifiPassword || wifiPassword.isEmpty()) {
                request->send(503, TEXT_PLAN, "wiFiPassword can't be null");
            } else {
                LOGGER.info("   saving...");

                NetworkSettings networkSettings;
                wifiSSID.toCharArray(&networkSettings.ssid[0], sizeof(networkSettings.ssid));
                wifiPassword.toCharArray(&networkSettings.password[0],
                                         sizeof(networkSettings.password));
                request->send(200, TEXT_PLAN, OK_RESPONSE);

                settingsManager->getNavigator()->saveNetworkSettingsAndRestart(&networkSettings);
                LOGGER.info("   saved");
            }
        } else {
            String message = "Read operation for \"wifi\" block unsupported. Use read for every parameter";
            LOGGER.error(message);
            request->send(200, TEXT_PLAN, message);
        }
    } else if (parameter == "DS18D20_LIST") {

    } else {
        if (operation == PARAMETER_OPERATION_READ) {
            String message = settingsManager->getNavigator()->getSettingByName(parameter);
            request->send(200, TEXT_PLAN, message);
        } else {
            // Запись параметров

            //  Подсчет кол-ва параметров
            int paramsCount = 1;
            int indexOfParamSeparator = -1;
            while ((indexOfParamSeparator = parameter.indexOf('|', indexOfParamSeparator + 1)) >= 0)
                paramsCount++;

            String params[paramsCount];

            // Соберем массивы
            indexOfParamSeparator = parameter.indexOf('|');
            int startCopyIndex = 0;
            int paramIndex = 0;
            if (parameter.indexOf('|') >= 0)
                do {
                    params[paramIndex++] = indexOfParamSeparator == -1
                                           ? parameter.substring(startCopyIndex)
                                           : parameter.substring(startCopyIndex, indexOfParamSeparator);
                    startCopyIndex = indexOfParamSeparator + 1;
                } while ((indexOfParamSeparator = parameter.indexOf('|', indexOfParamSeparator + 1)) >= 0);
            // последний элемент
            if (startCopyIndex < parameter.length()) {
                params[paramIndex] = indexOfParamSeparator == -1
                                     ? parameter.substring(startCopyIndex)
                                     : parameter.substring(startCopyIndex, indexOfParamSeparator);
            }

            for (int i = 0; i < paramsCount; LOGGER.debug(params[i++]));

            String message = settingsManager->getNavigator()->saveSettingsByNames(&params[0], paramsCount);

            if (message.isEmpty()) {
                request->send(200, TEXT_PLAN, OK_RESPONSE);
            } else {
                request->send(503, TEXT_PLAN, message);
                LOGGER.error(message);
            }

        }
    }

}
//----------------------------------------------------------------------

void WebServerController::startModelling(AsyncWebServerRequest *request) {
    if (heaterController)
        heaterController->startModelling();
}
//----------------------------------------------------------------------

void WebServerController::stopModelling(AsyncWebServerRequest *request) {
    if (heaterController)
        heaterController->stopModelling();
}
//----------------------------------------------------------------------

void WebServerController::getTelemetry(AsyncWebServerRequest *request) {
    if (heaterController) {
        TelemetryDataRecord *dataRecord = heaterController->getTelemetry();

        char *json = (char *) malloc(2048);

        sprintf(json, "{"
                        "\"date_time_ms\":\"%d\", \"interval_ms\": \"%d\","
                        "\"temperature\":{"
                        "\"core\": \"%00.2f\", \"input_t\": \"%00.2f\", "
                        "\"output_t\": \"%00.2f\", \"acc_top\": \"%00.2f\", "
                        "\"acc_upper\": \"%00.2f\", \"acc_low\": \"%00.2f\", "
                        "\"acc_bottom\": \"%00.2f\","
                        "\"forward_to_home\": \"%00.2f\", \"backward_from_home\": \"%00.2f\","
                        "\"core_sma_diff\":\"%00.2f\""
                        "}, "
                        "\"pumps\": {"
                        "\"pump_1_on\":%s, "
                        "\"pump_2_on\":%s, "
                        "\"core_flow\": \"%00.2f\" "
                        "}, "
                        "\"core_power\": \"%00.0f\", "
                        "\"doors\":{"
                        "\"smoke\": \"%00.2f\", \"oxygen\": \"%00.2f\", "
                        "\"upper\": \"%00.2f\" "
                        "},"
                        "\"pid\":{"
                        "\"p\":\"%00.2f\", \"i\":\"%00.2f\", \"d\":\"%00.2f\", "
                        "\"output\": \"%00.2f\""
                        "},"
                        "\"mode\":\"%d\""
                        "}"
                , dataRecord->date_time_ms, dataRecord->interval_ms
                , dataRecord->core_temp_sma, dataRecord->input_temp_sma
                , dataRecord->output_temp_sma, dataRecord->accumulator_top_temp_sma
                , dataRecord->accumulator_higher_temp_sma, dataRecord->accumulator_lower_temp_sma
                , dataRecord->accumulator_bottom_temp_sma
                , dataRecord->forwar_flow_temp_sma, dataRecord->backward_flow_temp_sma
                , dataRecord->core_SMA_diff_tempr
                , dataRecord->pump_1_state ? "true" : "false"
                , dataRecord->pump_2_state ? "true" : "false"
                , dataRecord->core_flow, dataRecord->core_EMA_power
                , dataRecord->smoke_door_position, dataRecord->oxygen_door_position
                , dataRecord->upper_door_position
                , dataRecord->pid_p, dataRecord->pid_i, dataRecord->pid_d
                , dataRecord->pid_output
                , dataRecord->heaterMode
        );

        request->send(200, TEXT_JSON, json);
        free(json);
    } else
        request->send(503, "Heater controller is not ready");
}
//----------------------------------------------------------------------

void WebServerController::loadFileByUrl(AsyncWebServerRequest *request) {
    String url = request->url();
    String mime;

    FS *fs;

    if (!SPIFFS.exists(request->url())) {

        if (!LOGGER.isSdPresents() || !SD.exists(request->url())) {
            LOGGER.error("url not found: \"" + request->url() + "\"");

            request->send(404, TEXT_PLAN, "not found for this");
            return;
        } else
            fs = &SD;
    } else
        fs = &SPIFFS;

    if (url.endsWith(".html")) {
        mime = "text/html";
    } else if (url.endsWith(".css")) {
        mime = "text/css";
    } else if (url.endsWith(".js")) {
        mime = "text/js";
    } else if (url.endsWith(".jpg")) {
        mime = "image/jpeg";
    } else {
        mime = TEXT_PLAN;
    }

    request->send(*fs, url, mime);
}
//----------------------------------------------------------------------


String WebServerController::systemSettingsProcessor(const String &paramName) {
    return settingsManager->getNavigator()->getSettingByName(paramName);
}
//----------------------------------------------------------------------
