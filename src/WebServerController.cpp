//
// Created by dsporykhin on 19.04.20.
//

#include "WebServerController.h"
#include <LittleFS.h>
#include <Update.h>
#include <esp_task_wdt.h>
#include "Defines.h"
#include "Logger.h"

String TEXT_PLAN = "text/plan";
String TEXT_JSON = "text/json";
String OK_RESPONSE = "OK";
String OK_RESPONSE_JSON = "{\"status\":0}";

String PROFILES_PARAMETER_ATTR_NAME = "parameter";

SettingsManager *WebServerController::settingsManager;
HeaterController *WebServerController::heaterController;
TouchDisplayController *WebServerController::touchDisplayController;
bool WebServerController::otaInProgress = false;
int WebServerController::otaCommand = U_FLASH;

void WebServerController::setTouchDisplayController(TouchDisplayController *touchDisplayController) {
    this->touchDisplayController = touchDisplayController;
}

WebServerController::WebServerController(SettingsManager *settingsManager) {
    WebServerController::settingsManager = settingsManager;
    this->heaterController = nullptr;

    LOGGER.info(" Starting web server...");

    webServer = new AsyncWebServer(80);

    webServer->on("/", HTTP_GET, [](AsyncWebServerRequest *request) {
        request->send(LittleFS, "/index.html", String(), false, systemSettingsProcessor);
    });

    webServer->on("/index.html", HTTP_GET, [](AsyncWebServerRequest *request) {
        request->send(LittleFS, "/index.html", String(), false, systemSettingsProcessor);
    });

    webServer->on("/settings.html", HTTP_GET, [](AsyncWebServerRequest *request) {
        request->send(LittleFS, "/settings.html", String(), false, systemSettingsProcessor);
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
    webServer->on("/manualWarmControl", HTTP_GET | HTTP_POST, manualWarmControl);
//    webServer->on("/manualWarmControl", HTTP_POST, manualWarmControl);
    webServer->on("/getTelemetry", HTTP_GET, getTelemetry);
    webServer->on("/reboot", HTTP_POST, reboot);

    webServer->on("/update/firmware", HTTP_POST, handleOtaRequest, handleOtaUpload);
    webServer->on("/update/code", HTTP_POST, handleOtaRequest, handleOtaUpload);
    webServer->on("/update/data", HTTP_POST, handleOtaRequest, handleOtaUpload);

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
    if (operation == "dump") {
        request->send(200, "application/json", settingsManager->getNavigator()->dumpAllSettingsJson());
        return;
    }

    String parameter = request->arg(parameterAttrName);

    if (!parameter || parameter.isEmpty()) {
        String message = "\"" + parameterAttrName + "\" can't be null";
        request->send(200, TEXT_PLAN, message);
    } else if (parameter == "wifi") {
        if (operation == PARAMETER_OPERATION_WRITE) {
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
    if (heaterController) {
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
        touchDisplayController->dirty();
    } else
        request->send(200, TEXT_JSON, "{\"status\":0, \"error\":\"heaterController is not initialized\"}");

}
//----------------------------------------------------------------------

void WebServerController::closeDoor(AsyncWebServerRequest *request) {
    if (heaterController) {
        heaterController->closeOxygenDoor();
        request->send(200, TEXT_JSON, OK_RESPONSE_JSON);
        touchDisplayController->dirty();
    } else
        request->send(200, TEXT_JSON, "{\"status\":0, \"error\":\"heaterController is not initialized\"}");

}
//----------------------------------------------------------------------

void WebServerController::manualWarmControl(AsyncWebServerRequest *request) {
    heaterController->postCounter = true;
    if (request->arg("action") == "open")
        openDoorFor15Min(request);
    else if (request->arg("action") == "close")
        closeDoor(request);
    else {
        request->send(200, TEXT_JSON, ((heaterController) && (heaterController->isOxygenDoorOpenedForATime())) ?
                  "{\"status\":0, \"opened\": \"true\"}": "{\"status\":0, \"opened\": \"false\"}");
    }
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

    if (!LittleFS.exists(request->url())) {
        LOGGER.error("url not found: \"" + request->url() + "\"");
        request->send(404, TEXT_PLAN, "not found for this");
        return;
    }

    if (url.endsWith(".html")) {
        mime = "text/html";
    } else if (url.endsWith(".css")) {
        mime = "text/css";
    } else if (url.endsWith(".js")) {
        mime = "application/javascript";
    } else if (url.endsWith(".jpg")) {
        mime = "image/jpeg";
    } else {
        mime = TEXT_PLAN;
    }

    request->send(LittleFS, url, mime);
}
//----------------------------------------------------------------------


String WebServerController::systemSettingsProcessor(const String &paramName) {
    return settingsManager->getNavigator()->getSettingByName(paramName);
}
//----------------------------------------------------------------------

bool WebServerController::isOtaInProgress() {
    return otaInProgress;
}
//----------------------------------------------------------------------

void WebServerController::handleOtaUpload(AsyncWebServerRequest *request, const String &filename, size_t index,
                                          uint8_t *data, size_t len, bool final) {
    (void) filename;
    esp_task_wdt_reset();
    if (index == 0) {
        otaCommand = request->url().endsWith("/data") ? U_SPIFFS : U_FLASH;
        otaInProgress = true;
        LOGGER.info("HTTP OTA start, partition=" + String(otaCommand));
        if (!Update.begin(UPDATE_SIZE_UNKNOWN, otaCommand)) {
            LOGGER.error("HTTP OTA begin failed");
            otaInProgress = false;
            return;
        }
    }
    if (len > 0 && Update.write(data, len) != len) {
        LOGGER.error("HTTP OTA write failed");
    }
    if (final) {
        if (!Update.end(true)) {
            LOGGER.error("HTTP OTA end failed");
            otaInProgress = false;
        }
    }
}
//----------------------------------------------------------------------

void WebServerController::handleOtaRequest(AsyncWebServerRequest *request) {
    if (Update.hasError() || !otaInProgress) {
        String errorMessage = Update.hasError() ? Update.errorString() : "Upload failed";
        LOGGER.error("HTTP OTA failed: " + errorMessage);
        Update.abort();
        otaInProgress = false;
        request->send(500, TEXT_PLAN, errorMessage);
        return;
    }
    LOGGER.info("HTTP OTA finished");
    request->send(200, TEXT_PLAN, OK_RESPONSE);
    delay(400);
    ESP.restart();
}
//----------------------------------------------------------------------

void WebServerController::reboot(AsyncWebServerRequest *request) {
    LOGGER.info("HTTP reboot requested");
    request->send(200, TEXT_PLAN, OK_RESPONSE);
    delay(400);
    ESP.restart();
}
//----------------------------------------------------------------------
