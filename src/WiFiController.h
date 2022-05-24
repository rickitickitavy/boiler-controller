//
// Created by dsporykhin on 19.04.20.
//

#include <lib/NTPClient/NTPClient.h>
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

//    String getTextErrorStatus();

//    void tryToConnect();

    void initNTP();

    WiFiUDP *ntpUDP;
public:
    NTPClient *timeClient;
    
public:
    WiFiController(SettingsManager *settingsManager);

    void checkConnection();
    void setApMode(IPAddress *ipAddress);
    bool isClientConnected();
};

extern WiFiController *wiFiController;
#endif //EFLAMEESP8266_WIFICONTROLLER_H
