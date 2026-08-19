#ifndef BOILERCONTROLLER_MQTTCONTROLLER_H
#define BOILERCONTROLLER_MQTTCONTROLLER_H

#include <WiFi.h>
#include <lib/mqtt/PubSubClient.h>

class SettingsManager;

class MqttController {
private:
    SettingsManager *settingsManager;
    WiFiClient wifiClient;
    PubSubClient mqtt;
    unsigned long last_reconnect_attempt_ms;
    bool force_reconnect;

    void ensureDisconnected();
    bool tryConnect();
    bool isConnectedToRouter();

public:
    explicit MqttController(SettingsManager *settingsManager);

    void handle();
    bool isConnected();
    void reloadFromSettings();
};

#endif
