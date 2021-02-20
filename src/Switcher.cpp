//
// Created by dsporykhin on 20.02.21.
//

#include "Switcher.h"
#include "Logger.h"

Switcher *Switcher::instance;


Switcher::Switcher(GlobalSettings *settings, MqttClient *mqttClient) {
    this->mqttClient = mqttClient;
    this->settings = settings;
    lastTimeOfStateHasBeenChanged = 0;
    instance = this;

    mqttClient->setMessageCallback(messageReceived);
    mqttClient->setConnectedCallback(mqttConnected);

    state = 100;
    checkState();

    pinMode(13, OUTPUT);
    digitalWrite(13, 0);
    controlPinLevel = false;
}

void Switcher::checkState() {
    // work only if more than ANTI_BUZZLE_INTERVAL_MS ms up from last change of state
    if ((lastTimeOfStateHasBeenChanged == 0)
        || ((millis() - lastTimeOfStateHasBeenChanged) > ANTI_BUZZLE_INTERVAL_MS)){
        bool newState = system_adc_read() >= HI_LEVEL_VALUE;
        if ((lastTimeOfStateHasBeenChanged == 0) || (newState != state)) {
            LOGGER.info("   state changed to " + String(newState));
            lastTimeOfStateHasBeenChanged = millis();
            state = newState;
            mqttClient->sendToStateTopic(state ? "ON" : "OFF");
        }
    }
}

void Switcher::dispatch() {
    checkState();
}

void Switcher::messageReceived(char *topic, uint8_t *payload, unsigned int length) {
    LOGGER.info("Message arrived to switcher");
    LOGGER.info(topic);

    char *data = (char *) malloc(length + 1);
    memcpy(data, payload, length);
    data[length] = 0;
    String text = String(data);

    if (text == "ON"){
        if (instance->state){
            instance->controlPinLevel = !instance->controlPinLevel;
            LOGGER.info("turning ON. New control = " + String(instance->controlPinLevel));
            digitalWrite(13, instance->controlPinLevel);
        }
    } else if (text == "OFF"){
        if (!instance->state){
            instance->controlPinLevel = !instance->controlPinLevel;
            LOGGER.info("turning OFF. New control = " + String(instance->controlPinLevel));
            digitalWrite(13, instance->controlPinLevel);
        }
    }

    free(data);
    LOGGER.info(text);
}

void Switcher::mqttConnected() {
    LOGGER.info("   switcher connected");
    instance->mqttClient->sendToStateTopic(instance->state ? "ON" : "OFF");
}