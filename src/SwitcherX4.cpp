//
// Created by dsporykhin on 20.02.21.
//

#include <backward/strstream>
#include "SwitcherX4.h"
#include "Logger.h"

SwitcherX4 *SwitcherX4::instance;


SwitcherX4::SwitcherX4(GlobalSettings *settings, MqttClient *mqttClient) {
    this->switcher_pins[0] = SW_PIN0;
    this->switcher_pins[1] = SW_PIN1;
    this->switcher_pins[2] = SW_PIN2;
    this->switcher_pins[3] = SW_PIN3;
    this->mqttClient = mqttClient;
    this->settings = settings;
    lastTimeOfStateHasBeenChanged = 0;
    lastTimeOfScanState = 0;
    instance = this;

    mqttClient->setMessageCallback(messageReceived);
    mqttClient->setConnectedCallback(mqttConnected);

    state = 100;

    for (int index = 0; index < 4; index++) {
        pinMode(switcher_pins[index], OUTPUT);
        digitalWrite(switcher_pins[index], LOW);
    }

    pinMode(A0, INPUT);
}


void SwitcherX4::reportStateToServer() {
    LOGGER.info("   sending state to server...");
    mqttClient->sendToStateTopic(state ? "ON" : "OFF");
}


void SwitcherX4::messageReceived(char *topic, uint8_t *payload, unsigned int length) {
    LOGGER.info("Message arrived to switcher");
    LOGGER.info(topic);

    int position;
    for (position = 0; topic[position]; position++) {
        if (topic[position] == '/')
            break;
    }

    LOGGER.info(String(length) + " = " + String(position));

    int channel;
    if (position != length) {
        channel = atoi(String(topic[position - 1]).c_str());
        LOGGER.info("channel " + String(channel));
        if (channel > 3 || channel < 0)
            return;
    } else {
        LOGGER.info("not found");
    }

    String topicStr = String(topic);
    if (topicStr == String(instance->settings->mqttServerBornTopic)) {
        LOGGER.info("   server online");
        instance->reportStateToServer();
    } else {
        char *data = (char *) malloc(length + 1);
        memcpy(data, payload, length);
        data[length] = 0;
        String text = String(data);

        if (text == "ON") {

            LOGGER.info("turning ON. New control = 1");
            digitalWrite(instance->switcher_pins[channel], HIGH);

            instance->reportStateToServer();
        } else if (text == "OFF") {

            LOGGER.info("turning OFF. New control = 0");
            digitalWrite(instance->switcher_pins[channel], LOW);

            instance->reportStateToServer();
        }

        instance->mqttClient->sendToStateTopic(channel, text);

        free(data);
        LOGGER.info(text);
    }
}


void SwitcherX4::mqttConnected() {
    LOGGER.info("   switcher connected");
    instance->reportStateToServer();
}