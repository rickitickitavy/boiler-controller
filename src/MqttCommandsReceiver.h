//
// Created by dsporykhin on 20.02.21.
//

#ifndef BASE_ESP8266_MQTT_COMMANDS_RECEIVER
#define BASE_ESP8266_MQTT_COMMANDS_RECEIVER

#define HI_LEVEL_VALUE 600
#define ANTI_BUZZLE_INTERVAL_MS 800

#include "MqttClient.h"
#include "Display.h"

class MqttCommandsReceiver {
private:
    GlobalSettings *settings;
    MqttClient *mqttClient;
    HeaterController *heaterController;


    static MqttCommandsReceiver *instance;

public:
    MqttCommandsReceiver(GlobalSettings *settings, MqttClient *mqttClient, HeaterController *heaterController);

    static void messageReceived(char* topic, uint8_t* payload, unsigned int length);

    static void mqttConnected();

    Display *display;
};


#endif //BASE_ESP8266_MQTT_COMMANDS_RECEIVER
