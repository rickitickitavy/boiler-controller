//
// Created by dsporykhin on 14.02.21.
//

#ifndef BASE_ESP8266_MQTT_MQTTCLIENT_H
#define BASE_ESP8266_MQTT_MQTTCLIENT_H
#define MQTT_CONNECTED_CALLBACK std::function<void(void)>
#define MQTT_MESSAGE_CALLBACK std::function<void(char*, uint8_t*, unsigned int)>

#include "lib/mqtt/PubSubClient.h"
#include "lib/ESPAsyncWebServer/ESPAsyncWebServer.h"
#include "GlobalSettings.h"
#include "HeaterController.h"

class MqttClient {
private:
    int port;
    String server;
    WiFiClient *espClient;
    PubSubClient *client;
    GlobalSettings *settings;

    MQTT_CONNECTED_CALLBACK connectedCallback;

    long lastReconnectTime;
    long lastCheckTime;
    static void defaultMsgCallback(char *topic, byte *payload, unsigned int length);

    void reconnect();

    void checkConnection();

public:

    MqttClient(GlobalSettings *settings);

    void setConnectedCallback(MQTT_CONNECTED_CALLBACK connectedCallback);

    void setMessageCallback(MQTT_MESSAGE_CALLBACK messageCallback);

    bool sendToStateTopic(byte *payload, int len);

    bool sendToCustomTopic(String topic, String payload);

    bool sendToStateTopic(const char *payload);

    void dispatch();
};


#endif //BASE_ESP8266_MQTT_MQTTCLIENT_H
