//
// Created by dsporykhin on 20.02.21.
//

#ifndef BASE_ESP8266_MQTT_SWITCHERX4_H
#define BASE_ESP8266_MQTT_SWITCHERX4_H

#define HI_LEVEL_VALUE 600
#define ANTI_BUZZLE_INTERVAL_MS 800

#include "MqttClient.h"

#define SW_PIN0 4
#define SW_PIN1 13
#define SW_PIN2 33
#define SW_PIN3 32

class SwitcherX4 {
private:
    bool state;
    long lastTimeOfStateHasBeenChanged;
    long lastTimeOfScanState;
    GlobalSettings *settings;
    MqttClient *mqttClient;
    int switcher_pins[4];

    static SwitcherX4 *instance;

    void reportStateToServer();
public:
    SwitcherX4(GlobalSettings *settings, MqttClient *mqttClient);

    static void messageReceived(char* topic, uint8_t* payload, unsigned int length);

    static void mqttConnected();
};


#endif //BASE_ESP8266_MQTT_SWITCHERX4_H
