//
// Created by dsporykhin on 14.02.21.
//

#include "MqttClient.h"
#include "Logger.h"
#include "Defines.h"

MqttClient::MqttClient(GlobalSettings *settings) {
    this->server = server;
    this->port = port;
    this->settings = settings;
    espClient = new WiFiClient();
    client = new PubSubClient(*espClient);
    client->setServer(server.c_str(), port);
    client->setCallback(callback);
    lastReconnectTime = 0;
}

void MqttClient::checkConnection() {

}

void MqttClient::reconnect() {
    if ((!client->connected()) && ((millis() - lastReconnectTime > settings->mqttReconnectIntervalMs) || (lastReconnectTime == 0))) {
        LOGGER.info("Attempting MQTT connection...");
        // Attempt to connect
        String deviceName = String(settings->mqttDeviceName[0]);
        if (client->connect(deviceName.c_str())) {
            Serial.println("connected");
            // Once connected, publish an announcement...
            client->publish(LOGIN_TOPIC, "1");
            // ... and resubscribe
            client->subscribe((INCOME_COMMAND_TOPIC + deviceName).c_str());
        } else {
            LOGGER.error("connection failed, state=");
            Serial.print(client->state());
            Serial.println(" try again in 5 seconds");
            // Wait 5 seconds before retrying
            delay(5000);
        }
    }
}

void MqttClient::callback(char *topic, byte *payload, unsigned int length) {
    LOGGER.debug("Message arrived");
    LOGGER.debug(topic);
    for (int i = 0; i < length; i++) {
        Serial.print((char)payload[i]);
    }
    Serial.println();
}