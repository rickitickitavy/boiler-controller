//
// Created by dsporykhin on 14.02.21.
//

#include "MqttClient.h"
#include "Logger.h"

MqttClient::MqttClient(GlobalSettings *settings) {
    this->settings = settings;
    this->connectedCallback = connectedCallback;

    this->server = String(settings->mqttServer);
    this->port = settings->mqttPort;

    espClient = new WiFiClient();
    client = new PubSubClient(*espClient);
    client->setCallback(defaultMsgCallback);
    client->setServer(server.c_str(), port);

    lastReconnectTime = 0;
    lastCheckTime = 0;
}
//-------------------------------------------------------------

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
//-------------------------------------------------------------

void MqttClient::reconnect() {
    if (!client->connected()) {
        LOGGER.info("Attempting MQTT connection...");
        // Attempt to connect
        String deviceInputTopic = String(settings->deviceIncomingCommandTopicPrefix) + "/" +  String(settings->mqttDeviceName);
        if (client->connect(settings->mqttDeviceName)) {
            // Once connected, publish an announcement...
            if (this->connectedCallback  != nullptr){
                this->connectedCallback();
            }
            LOGGER.info("   MQTT connected. Subscribing to '" + deviceInputTopic + "'");
            LOGGER.info(client->subscribe(deviceInputTopic.c_str())
                        ? "   subscribed "
                        : "   NOT subscribed");
            LOGGER.info("   MQTT connected. Subscribing to '" + String(settings->mqttServerBornTopic) + "'");
            LOGGER.info(client->subscribe(settings->mqttServerBornTopic)
                        ? "   subscribed "
                        : "   NOT subscribed");
            LOGGER.info("   sending 'have born' message...");
            client->publish(settings->deviceIHaveBornTopic, settings->mqttDeviceName);
            // ... and resubscribe
        } else {
            LOGGER.error("connection failed, state=" + String(client->state()));
        }
        LOGGER.info("   mqtt done");
    }
}
//-------------------------------------------------------------

void MqttClient::defaultMsgCallback(char *topic, byte *payload, unsigned int length) {
    LOGGER.info("Message arrived");
    LOGGER.info(topic);
    char *data = (char *) malloc(length + 1);
    memcpy(data, payload, length);
    data[length] = 0;
    String text = String(data);
    free(data);
    LOGGER.info(text);
}
//-------------------------------------------------------------

void MqttClient::dispatch() {
    checkConnection();
    if (client->connected()) {
        client->loop();
    }
}
//-------------------------------------------------------------

void MqttClient::setConnectedCallback(MQTT_CONNECTED_CALLBACK connectedCallback) {
    connectedCallback = connectedCallback;
}
//-------------------------------------------------------------

void MqttClient::setMessageCallback(MQTT_MESSAGE_CALLBACK messageCallback) {
    client->setCallback(messageCallback);
}
//-------------------------------------------------------------

bool MqttClient::sendToStateTopic(byte *payload, int len) {
    lastCheckTime = 0;
    checkConnection();
    if (client->connected()){
        String outTopic = String(settings->deviceStateOutgoingTopicPrefix) + "/" + String(settings->mqttDeviceName);
        client->publish(outTopic.c_str(), payload, len);
        return true;
    } else {
        return false;
    }
}
//-------------------------------------------------------------

bool MqttClient::sendToStateTopic(String payload) {
    lastCheckTime = 0;
    checkConnection();
    if (client->connected()){
        String outTopic = String(settings->deviceStateOutgoingTopicPrefix) + "/" + String(settings->mqttDeviceName);
        client->publish(outTopic.c_str(), payload.c_str());
        return true;
    }
    return false;
}
//-------------------------------------------------------------

bool MqttClient::sendToCustomTopic(String topic, String payload) {
    lastCheckTime = 0;
    checkConnection();
    if (client->connected()){
        String outTopic = topic + "/" + String(settings->mqttDeviceName);
        client->publish(outTopic.c_str(), payload.c_str());
        return true;
    }
    return false;
}
//-------------------------------------------------------------

bool MqttClient::sendToStateTopic(const char *payload) {
    lastCheckTime = 0;
    checkConnection();
    if (client->connected()){
        String outTopic = String(settings->deviceStateOutgoingTopicPrefix) + "/" + String(settings->mqttDeviceName);
        client->publish(outTopic.c_str(), payload);
        return true;
    }
    return false;
}
//-------------------------------------------------------------

