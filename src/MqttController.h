#ifndef BOILERCONTROLLER_MQTTCONTROLLER_H
#define BOILERCONTROLLER_MQTTCONTROLLER_H

#include <WiFi.h>
#include <lib/mqtt/PubSubClient.h>
#include <functional>

class SettingsManager;
class HeaterController;
struct GlobalSettings;

class MqttController {
private:
    SettingsManager *settingsManager;
    HeaterController *heaterController;
    WiFiClient wifiClient;
    PubSubClient mqtt;
    unsigned long last_reconnect_attempt_ms;
    unsigned long last_state_publish_ms;
    bool force_reconnect;
    char state_topic[96];
    char command_topic[96];
    std::function<void(char *, uint8_t *, unsigned int)> external_callback;

    void ensureDisconnected();
    bool tryConnect();
    bool isConnectedToRouter();
    void buildTopics(GlobalSettings *globalSettings);
    void publishState();
    void handleCommandPayload(const char *payload);

public:
    explicit MqttController(SettingsManager *settingsManager);

    void setHeaterController(HeaterController *controller);
    void setCallback(std::function<void(char *, uint8_t *, unsigned int)> callback);
    void onMessage(char *topic, uint8_t *payload, unsigned int length);

    void handle();
    bool isConnected();
    void reloadFromSettings();
};

#endif
