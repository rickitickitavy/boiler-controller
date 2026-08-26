//
// Created by dsporykhin on 19.04.20.
//

#ifndef EFLAMEESP8266_WEBSERVERCONTROLLER_H
#define EFLAMEESP8266_WEBSERVERCONTROLLER_H

#define PARAMETER_OPERATION_WRITE "write"
#define PARAMETER_OPERATION_READ "read"

#define PROFILES_OPERATION_ATTR_NAME "operation"
#define PROFILES_NAME_ATTR_NAME "profileName"
#define PROFILES_OPERATION_GET_LIST "getList"
#define PROFILES_OPERATION_ACTIVATE "activate"
#define PROFILES_OPERATION_SAVE "save"
#define PROFILES_OPERATION_REMOVE "remove"

#include <ESPAsyncWebServer.h>
#include "SettingsManager.h"
#include "HeaterController.h"
#include "TouchDisplayController.h"

class WebServerController {
private:
    AsyncWebServer* webServer;
    static TouchDisplayController *touchDisplayController;
    static bool otaInProgress;
    static int otaCommand;

    static void handleOtaRequest(AsyncWebServerRequest *request);
    static void handleOtaUpload(AsyncWebServerRequest *request, const String &filename, size_t index, uint8_t *data,
                                size_t len, bool final);
public:
    static SettingsManager* settingsManager;
    static HeaterController *heaterController;

    WebServerController(SettingsManager* settingsManager);

    void setTouchDisplayController(TouchDisplayController *touchDisplayController);

    static bool isOtaInProgress();

    static String systemSettingsProcessor(const String& paramName);

    static void settingsApiProcessor(AsyncWebServerRequest *request);

    static void loadFileByUrl(AsyncWebServerRequest *request);

#ifdef ENABLE_MODELLING
    static void startModelling(AsyncWebServerRequest *request);

    static void stopModelling(AsyncWebServerRequest *request);
#endif

    static void openDoorFor15Min(AsyncWebServerRequest *request);

    static void closeDoor(AsyncWebServerRequest *request);

    static void manualWarmControl(AsyncWebServerRequest *request);

    static void getTelemetry(AsyncWebServerRequest *request);

    static void reboot(AsyncWebServerRequest *request);

    static void getLog(AsyncWebServerRequest *request);

};

extern String TEXT_PLAN;
#endif //EFLAMEESP8266_WEBSERVERCONTROLLER_H
