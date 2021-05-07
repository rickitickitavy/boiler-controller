//
// Created by dsporykhin on 20.02.21.
//

#ifndef BASE_ESP8266_MQTT_SWITCHER_H
#define BASE_ESP8266_MQTT_SWITCHER_H

#define HI_LEVEL_VALUE 600
#define ANTI_BUZZLE_INTERVAL_MS 800
#define STATUS_PIN_READ A0
#define SWITCHER_PIN 13

#include "MqttClient.h"


class Switcher {
private:
    bool state;
    long lastTimeOfStateHasBeenChanged;
    long lastTimeOfScanState;
    GlobalSettings *settings;
    MqttClient *mqttClient;
    bool controlPinLevel;

    static Switcher *instance;

    void checkState();

    void reportStateToServer();
public:
    Switcher(GlobalSettings *settings, MqttClient *mqttClient);


    void dispatch();


    static void messageReceived(char* topic, uint8_t* payload, unsigned int length);

    static void mqttConnected();
};


#endif //BASE_ESP8266_MQTT_SWITCHER_H
