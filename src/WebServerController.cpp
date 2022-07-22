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
String OK_RESPONSE_JSON = "{\"status\":0}";

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
    webServer->on("/openDoorFor15Min", HTTP_GET, openDoorFor15Min);
    webServer->on("/closeDoor", HTTP_GET, closeDoor);
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
    if (heaterController){
        heaterController->startModelling();
        request->send(200, TEXT_JSON, OK_RESPONSE_JSON);
    } else
        request->send(200, TEXT_JSON, "{\"status\":0, \"error\":\"heaterController is not initialized\"}");

}
//----------------------------------------------------------------------

void WebServerController::stopModelling(AsyncWebServerRequest *request) {
    if (heaterController) {
        heaterController->stopModelling();
        request->send(200, TEXT_JSON, OK_RESPONSE_JSON);
    } else
        request->send(200, TEXT_JSON, "{\"status\":0, \"error\":\"heaterController is not initialized\"}");
}
//----------------------------------------------------------------------

void WebServerController::openDoorFor15Min(AsyncWebServerRequest *request) {
    if (heaterController) {
        heaterController->openOxygenDoorForTime(900);
        request->send(200, TEXT_JSON, OK_RESPONSE_JSON);
    } else
        request->send(200, TEXT_JSON, "{\"status\":0, \"error\":\"heaterController is not initialized\"}");

}
//----------------------------------------------------------------------

void WebServerController::closeDoor(AsyncWebServerRequest *request) {
    if (heaterController) {
        heaterController->closeOxygenDoor();
        request->send(200, TEXT_JSON, OK_RESPONSE_JSON);
    } else
        request->send(200, TEXT_JSON, "{\"status\":0, \"error\":\"heaterController is not initialized\"}");

}
//----------------------------------------------------------------------

void WebServerController::getTelemetry(AsyncWebServerRequest *request) {
    if (heaterController) {
        char *json = (char *) malloc(2048);
        heaterController->getTelemetry(json);
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
