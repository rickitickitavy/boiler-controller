//
// Created by dsporykhin on 14.02.21.
//

#ifndef BASE_ESP8266_MQTT_MQTTCLIENT_H
#define BASE_ESP8266_MQTT_MQTTCLIENT_H
#define CONNECTED_CALLBACK std::function<void(void)>
#define MQTT_MESSAGE_CALLBACK std::function<void(char*, uint8_t*, unsigned int)>

#include "../lib/mqtt/PubSubClient.h"
#include "../lib/ESPAsyncWebServer/ESPAsyncWebServer.h"
#include "GlobalSettings.h"
#include <ESP8266WiFi.h>

class MqttClient {
private:
    int port;
    String server;
    WiFiClient *espClient;
    PubSubClient *client;
    GlobalSettings *settings;

    CONNECTED_CALLBACK connectedCallback;

    long lastReconnectTime;
    long lastCheckTime;
    static void callback(char* topic, byte* payload, unsigned int length);

    void reconnect();

    void checkConnection();

public:

    MqttClient(GlobalSettings *settings, CONNECTED_CALLBACK connectedCallback, MQTT_MESSAGE_CALLBACK messageCallback);

    void dispatch();
};


#endif //BASE_ESP8266_MQTT_MQTTCLIENT_H
