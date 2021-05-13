//
// Created by dsporykhin on 19.04.20.
//

#include "WebServerController.h"
#include "SettingsManager.h"

#ifndef EFLAMEESP8266_WIFICONTROLLER_H
#define EFLAMEESP8266_WIFICONTROLLER_H


class WiFiController {
private:
    SettingsManager *settingsManager;
    WebServerController *serverController;
    long lastConnectedTime;

    void init();

    String getTextErrorStatus();

    void tryToConnect();

public:
    WiFiController(SettingsManager *settingsManager);

    void checkConnection();

    bool isClientConnected();
};

extern WiFiController *wiFiController;
#endif //EFLAMEESP8266_WIFICONTROLLER_H
