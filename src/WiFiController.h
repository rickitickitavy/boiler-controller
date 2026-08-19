//
// Created by dsporykhin on 19.04.20.
//

#include <lib/NTPClient/NTPClient.h>
#include "WebServerController.h"
#include "SettingsManager.h"
#include "HeaterController.h"

#ifndef EFLAMEESP8266_WIFICONTROLLER_H
#define EFLAMEESP8266_WIFICONTROLLER_H


class WiFiController {
private:
    SettingsManager *settingsManager;
    WebServerController *serverController;
    long lastConnectedTime;

    void init();

    void initNTP();

    WiFiUDP *ntpUDP;
public:
    NTPClient *timeClient;
    
public:
    WiFiController(SettingsManager *settingsManager);

    void reapplyNetworkSettings();
    void checkConnection();
    void setApMode(IPAddress *ipAddress);
    bool isClientConnected();
    void setHeaterController(HeaterController *heaterController);
    WebServerController *getWebServerController();
};

extern WiFiController *wiFiController;
#endif //EFLAMEESP8266_WIFICONTROLLER_H
