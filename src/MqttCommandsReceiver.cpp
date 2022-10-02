//
// Created by dsporykhin on 20.02.21.
//

#include <backward/strstream>
#include "MqttCommandsReceiver.h"
#include "Logger.h"

MqttCommandsReceiver *MqttCommandsReceiver::instance;


MqttCommandsReceiver::MqttCommandsReceiver(GlobalSettings *settings, MqttClient *mqttClient, HeaterController *heaterController) {
    this->mqttClient = mqttClient;
    this->settings = settings;
    this->heaterController = heaterController;
    display = nullptr;
    instance = this;

    mqttClient->setMessageCallback(messageReceived);
//    mqttClient->setConnectedCallback(mqttConnected);
}

void MqttCommandsReceiver::messageReceived(char *topic, uint8_t *payload, unsigned int length) {
    LOGGER.info("Message received");
    char* buf = (char*)malloc(length + 1);
    memcpy(buf, payload, length);
    buf[length] = 0;
    LOGGER.info(topic);

    if (strcmp(buf, "ON") == 0) {
        LOGGER.info("Command for open");
        instance->heaterController->openOxygenDoorForTime(900);

    } else if (strcmp(buf, "OFF") == 0) {

        LOGGER.info("Command for close");
        instance->heaterController->closeOxygenDoor();
    }

    free(buf);
    instance->mqttClient->sendToStateTopic(instance->heaterController->isOxygenDoorOpenedForATime() ? "ON" : "OFF");

}


void MqttCommandsReceiver::mqttConnected() {
    LOGGER.info("   switcher connected");
}