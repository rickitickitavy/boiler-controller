//
// Created by dsporykhin on 14.02.21.
//

#include "MqttClient.h"
#include "Logger.h"
#include "Defines.h"

MqttClient::MqttClient(GlobalSettings *settings, CONNECTED_CALLBACK connectedCallback, MQTT_MESSAGE_CALLBACK messageCallback) {
    this->settings = settings;
    this->connectedCallback = connectedCallback;

    this->server = String(settings->mqttServer);
    this->port = settings->mqttPort;

    espClient = new WiFiClient();
    client = new PubSubClient(*espClient);
    if (messageCallback){
        client->setCallback(messageCallback);
    } else {
        client->setCallback(callback);
    }
    client->setServer(server.c_str(), port);

    lastReconnectTime = 0;
    lastCheckTime = 0;
}

void MqttClient::checkConnection() {
    if ((lastCheckTime == 0)
        || ((millis() - lastCheckTime) > 100)) {
        lastCheckTime = millis();
        if ((!client->connected()) &&
            ((millis() - lastReconnectTime > settings->mqttReconnectIntervalMs) || (lastReconnectTime == 0))) {
            lastReconnectTime = millis();
            reconnect();
        }
    }
}

void MqttClient::reconnect() {
    if (!client->connected()) {
        LOGGER.info("Attempting MQTT connection...");
        // Attempt to connect
        String deviceInputTopic = String(settings->deviceIncomingCommandTopicPrefix) + "/" +  String(settings->mqttDeviceName);
        if (client->connect(deviceInputTopic.c_str())) {
            LOGGER.info("   MQTT connected. Subscribing to '" + deviceInputTopic + "'");
            // Once connected, publish an announcement...
            if (this->connectedCallback){
                this->connectedCallback();
            }
            client->publish(settings->deviceIHaveBornTopic, settings->mqttDeviceName);
            // ... and resubscribe
            LOGGER.info(client->subscribe(deviceInputTopic.c_str())
                        ? "   subscribed "
                        : "   NOT subscribed");
        } else {
            LOGGER.error("connection failed, state=" + String(client->state()));
        }
    }
}

void MqttClient::callback(char *topic, byte *payload, unsigned int length) {
    LOGGER.info("Message arrived");
    LOGGER.info(topic);
    char *data = (char *) malloc(length + 1);
    memcpy(data, payload, length);
    data[length] = 0;
    String text = String(data);
    free(data);
    LOGGER.info(text);
}

void MqttClient::dispatch() {
    checkConnection();
    if (client->connected()) {
        client->loop();
    }
}