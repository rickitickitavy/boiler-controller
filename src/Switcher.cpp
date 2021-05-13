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
    lastTimeOfScanState = 0;
    instance = this;

    mqttClient->setMessageCallback(messageReceived);
    mqttClient->setConnectedCallback(mqttConnected);

    state = 100;
    checkState();

    pinMode(SWITCHER_PWM_PIN, OUTPUT);
    analogWrite(SWITCHER_PWM_PIN, 90);

    pinMode(SWITCHER_PIN, OUTPUT);
    digitalWrite(SWITCHER_PIN, 0);
    pinMode(A0, INPUT);
    controlPinLevel = false;
}

void Switcher::checkState() {
    // work only if more than ANTI_BUZZLE_INTERVAL_MS ms up from last change of state
    if ((lastTimeOfScanState == 0) || ((millis() - lastTimeOfScanState) > 50)) {
        if ((lastTimeOfStateHasBeenChanged == 0)
            || ((millis() - lastTimeOfStateHasBeenChanged) > ANTI_BUZZLE_INTERVAL_MS)) {
            lastTimeOfScanState = millis();
            int analog = analogRead(A0);
            delay(30);
//            int analog = digitalRead(12) ? 1024 : 0;
            bool newState = (analog >= HI_LEVEL_VALUE);
            if ((lastTimeOfStateHasBeenChanged == 0) || (newState != state)) {
                LOGGER.info("   state changed to " + String(newState));
                lastTimeOfStateHasBeenChanged = millis();
                state = newState;
                mqttClient->sendToStateTopic(state ? "ON" : "OFF");
            }
        }
    }
}

void Switcher::reportStateToServer() {
    LOGGER.info("   sending state to server...");
    mqttClient->sendToStateTopic(state ? "ON" : "OFF");
}

void Switcher::dispatch() {
    checkState();
}

void Switcher::messageReceived(char *topic, uint8_t *payload, unsigned int length) {
    LOGGER.info("Message arrived to switcher");
    LOGGER.info(topic);

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
            if (!instance->state) {
                instance->controlPinLevel = !instance->controlPinLevel;
                LOGGER.info("turning ON. New control = " + String(instance->controlPinLevel));
                digitalWrite(SWITCHER_PIN, instance->controlPinLevel ? HIGH : LOW);
            }
            instance->reportStateToServer();
        } else if (text == "OFF") {
            if (instance->state) {
                instance->controlPinLevel = !instance->controlPinLevel;
                LOGGER.info("turning OFF. New control = " + String(instance->controlPinLevel));
                digitalWrite(SWITCHER_PIN, instance->controlPinLevel ? HIGH : LOW);
            }
            instance->reportStateToServer();
        }

        free(data);
        LOGGER.info(text);
    }
}


void Switcher::mqttConnected() {
    LOGGER.info("   switcher connected");
    instance->reportStateToServer();
}