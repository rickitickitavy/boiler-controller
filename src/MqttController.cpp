#include "MqttController.h"
#include "SettingsManager.h"
#include "Logger.h"
#include <WiFi.h>

// Keep MQTT off the hot path: failed TCP/MQTT handshakes must not stall the UI/heater loop.
#define MQTT_TCP_CONNECT_TIMEOUT_MS 1000
#define MQTT_SOCKET_TIMEOUT_SEC 1
#define MQTT_MIN_RECONNECT_INTERVAL_MS 5000

MqttController::MqttController(SettingsManager *settingsManager)
        : settingsManager(settingsManager), mqtt(wifiClient),
          last_reconnect_attempt_ms(0), force_reconnect(true) {
    wifiClient.setConnectionTimeout(MQTT_TCP_CONNECT_TIMEOUT_MS);
    mqtt.setSocketTimeout(MQTT_SOCKET_TIMEOUT_SEC);
}

void MqttController::ensureDisconnected() {
    if (mqtt.connected())
        mqtt.disconnect();
}

bool MqttController::tryConnect() {
    if (!settingsManager)
        return false;
    GlobalSettings *globalSettings = settingsManager->getSettings();
    if (!globalSettings || globalSettings->mqttServer[0] == 0 || globalSettings->mqttDeviceName[0] == 0)
        return false;

    mqtt.setServer(globalSettings->mqttServer, (uint16_t) globalSettings->mqttPort);
    wifiClient.setConnectionTimeout(MQTT_TCP_CONNECT_TIMEOUT_MS);
    mqtt.setSocketTimeout(MQTT_SOCKET_TIMEOUT_SEC);

    if (mqtt.connect(globalSettings->mqttDeviceName)) {
        LOGGER.info(String("MQTT connected to ") + globalSettings->mqttServer);
        return true;
    }
    return false;
}

void MqttController::reloadFromSettings() {
    force_reconnect = true;
    ensureDisconnected();
    last_reconnect_attempt_ms = 0;
}

bool MqttController::isConnectedToRouter() {
    // Soft-AP fallback must not run MQTT; only STA linked to a router.
    return WiFi.getMode() == WIFI_STA && WiFi.isConnected();
}

bool MqttController::isConnected() {
    return isConnectedToRouter() && mqtt.connected();
}

void MqttController::handle() {
    if (!isConnectedToRouter()) {
        ensureDisconnected();
        return;
    }

    if (force_reconnect) {
        ensureDisconnected();
        force_reconnect = false;
        last_reconnect_attempt_ms = 0;
    }

    if (mqtt.connected()) {
        mqtt.loop();
        return;
    }

    unsigned long reconnectIntervalMs = MQTT_MIN_RECONNECT_INTERVAL_MS;
    if (settingsManager && settingsManager->getSettings()) {
        long configuredReconnectMs = settingsManager->getSettings()->mqttReconnectIntervalMs;
        if (configuredReconnectMs > (long) reconnectIntervalMs)
            reconnectIntervalMs = (unsigned long) configuredReconnectMs;
    }

    unsigned long nowMs = millis();
    if (last_reconnect_attempt_ms != 0 && (nowMs - last_reconnect_attempt_ms) < reconnectIntervalMs)
        return;

    last_reconnect_attempt_ms = nowMs;
    tryConnect();
}
