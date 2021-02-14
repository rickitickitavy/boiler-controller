//
// Created by dsporykhin on 14.02.21.
//

#ifndef BASE_ESP8266_MQTT_MQTTCLIENT_H
#define BASE_ESP8266_MQTT_MQTTCLIENT_H

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
    long lastReconnectTime;
    static void callback(char* topic, byte* payload, unsigned int length);

public:

    MqttClient(GlobalSettings *settings);

    void reconnect();

    void checkConnection();
};


#endif //BASE_ESP8266_MQTT_MQTTCLIENT_H
