#include "MqttController.h"
#include "SettingsManager.h"
#include "HeaterController.h"
#include "Logger.h"
#include <WiFi.h>
#include <math.h>
#include <string.h>
#include <limits.h>

// Keep MQTT off the hot path: failed TCP/MQTT handshakes must not stall the UI/heater loop.
#define MQTT_TCP_CONNECT_TIMEOUT_MS 1000
#define MQTT_SOCKET_TIMEOUT_SEC 1
#define MQTT_MIN_RECONNECT_INTERVAL_MS 5000
#define MQTT_STATE_BUFFER_SIZE 512

static const char *heaterModeEnglishNameForMqtt(uint8_t mode) {
    switch (mode) {
        case STAND_BY:
            return "Stand By";
        case WARMING:
            return "Warming";
        case FINAL_COOLING:
            return "Cooling";
        case PID:
            return "Burning";
        case OVERHEATED:
            return "Overheat";
        case CRITICAL:
            return "Critical";
        default:
            return "Unknown";
    }
}

/** Non-finite or outside [min,max] (incl. INT_MAX-scale garbage) → 0. */
static float mqttSaneOrZero(float value, float minValue, float maxValue) {
    if (!isfinite(value) || value < minValue || value > maxValue)
        return 0.0f;
    return value;
}

static int mqttSaneIntOrZero(float value, float minValue, float maxValue) {
    if (!isfinite(value) || value < minValue || value > maxValue)
        return 0;
    if (value >= (float) INT_MAX || value <= (float) INT_MIN)
        return 0;
    return (int) value;
}

MqttController::MqttController(SettingsManager *settingsManager)
        : settingsManager(settingsManager), heaterController(nullptr), mqtt(wifiClient),
          last_reconnect_attempt_ms(0), last_state_publish_ms(0), force_reconnect(true) {
    state_topic[0] = 0;
    command_topic[0] = 0;
    wifiClient.setConnectionTimeout(MQTT_TCP_CONNECT_TIMEOUT_MS);
    mqtt.setSocketTimeout(MQTT_SOCKET_TIMEOUT_SEC);
    mqtt.setBufferSize(MQTT_STATE_BUFFER_SIZE);
}

void MqttController::setHeaterController(HeaterController *controller) {
    heaterController = controller;
}

void MqttController::setCallback(std::function<void(char *, uint8_t *, unsigned int)> callback) {
    external_callback = callback;
    mqtt.setCallback([this](char *topic, uint8_t *payload, unsigned int length) {
        if (external_callback)
            external_callback(topic, payload, length);
        else
            onMessage(topic, payload, length);
    });
}

void MqttController::onMessage(char *topic, uint8_t *payload, unsigned int length) {
    (void) topic;
    if (!payload || length == 0)
        return;

    char command[32];
    unsigned int copyLength = length < sizeof(command) - 1 ? length : sizeof(command) - 1;
    memcpy(command, payload, copyLength);
    command[copyLength] = 0;
    handleCommandPayload(command);
}

void MqttController::handleCommandPayload(const char *payload) {
    if (!heaterController || !payload)
        return;

    if (strcmp(payload, "OPEN") == 0) {
        heaterController->openOxygenDoorForTime(900);
    } else if (strcmp(payload, "CLOSE") == 0) {
        heaterController->closeOxygenDoor();
    } else if (strcmp(payload, "PUMPS_ON") == 0) {
        heaterController->requestMqttPumpsOn();
    } else if (strcmp(payload, "PUMPS_OFF") == 0) {
        heaterController->requestMqttPumpsOff();
    }
}

void MqttController::ensureDisconnected() {
    if (mqtt.connected())
        mqtt.disconnect();
}

void MqttController::buildTopics(GlobalSettings *globalSettings) {
    snprintf(state_topic, sizeof(state_topic), "%s/%s",
             globalSettings->mqttDeviceName, globalSettings->deviceStateOutgoingTopicPrefix);
    snprintf(command_topic, sizeof(command_topic), "%s/%s",
             globalSettings->mqttDeviceName, globalSettings->deviceIncomingCommandTopicPrefix);
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

    bool useCredentials = globalSettings->mqttUser[0] != 0 && globalSettings->mqttPassword[0] != 0;
    bool connected = useCredentials
                     ? mqtt.connect(globalSettings->mqttDeviceName,
                                    globalSettings->mqttUser,
                                    globalSettings->mqttPassword)
                     : mqtt.connect(globalSettings->mqttDeviceName);
    if (!connected)
        return false;

    buildTopics(globalSettings);
    if (!mqtt.subscribe(command_topic)) {
        LOGGER.error(String("MQTT subscribe failed: ") + command_topic);
        ensureDisconnected();
        return false;
    }

    LOGGER.info(String("MQTT connected to ") + globalSettings->mqttServer);
    LOGGER.info(String("MQTT publish topic: ") + state_topic);
    LOGGER.info(String("MQTT subscribe topic: ") + command_topic);
    last_state_publish_ms = 0;
    return true;
}

void MqttController::publishState() {
    if (!heaterController || !mqtt.connected() || state_topic[0] == 0)
        return;

    TelemetryDataRecord *telemetry = heaterController->getTelemetryRecord();
    if (!telemetry)
        return;

    const char *modeName = heaterModeEnglishNameForMqtt(telemetry->heaterMode);
    char modeField[16];
    strncpy(modeField, modeName, sizeof(modeField) - 1);
    modeField[sizeof(modeField) - 1] = 0;

    int oxygenTimerSec = (int) heaterController->getOxygenDoorOpenedForATime();
    if (oxygenTimerSec < 0)
        oxygenTimerSec = 0;

    const char *pumpStatus = heaterController->isAnyPumpOn() ? "ON" : "OFF";
    unsigned long timeOnlineSec = millis() / 1000UL;

    char payload[MQTT_STATE_BUFFER_SIZE];
    int written = snprintf(
            payload, sizeof(payload),
            "{%s,%d,%s,%lu,%.2f,%.2f,%.2f,%.2f,%.2f,%.2f,%.2f,%.2f,%.2f,%.2f,%.2f,%.2f,%.2f,%d,%.2f,%.2f,%.2f,%.2f,%.2f,%.2f}",
            modeField,
            oxygenTimerSec,
            pumpStatus,
            timeOnlineSec,
            mqttSaneOrZero(telemetry->internal_temp, -20.0f, 200.0f),
            mqttSaneOrZero(telemetry->core_temp_sma, -20.0f, 200.0f),
            mqttSaneOrZero(telemetry->output_temp_sma, -20.0f, 200.0f),
            mqttSaneOrZero(telemetry->input_temp_sma, -20.0f, 200.0f),
            mqttSaneOrZero(telemetry->accumulator_top_temp_sma, -20.0f, 200.0f),
            mqttSaneOrZero(telemetry->accumulator_higher_temp_sma, -20.0f, 200.0f),
            mqttSaneOrZero(telemetry->accumulator_lower_temp_sma, -20.0f, 200.0f),
            mqttSaneOrZero(telemetry->accumulator_bottom_temp_sma, -20.0f, 200.0f),
            mqttSaneOrZero(telemetry->forwar_flow_temp_sma, -20.0f, 200.0f),
            mqttSaneOrZero(telemetry->backward_flow_temp_sma, -20.0f, 200.0f),
            mqttSaneOrZero(telemetry->oxygen_door_position, 0.0f, 100.0f),
            mqttSaneOrZero(telemetry->upper_door_position, 0.0f, 100.0f),
            mqttSaneOrZero(telemetry->smoke_door_position, 0.0f, 100.0f),
            mqttSaneIntOrZero(telemetry->core_EMA_power, -40000.0f, 200000.0f),
            mqttSaneOrZero(telemetry->power_balance_kwt_hour, -800.0f, 800.0f),
            mqttSaneOrZero(telemetry->accumulated_energy_kwt_hour, -800.0f, 800.0f),
            mqttSaneOrZero(telemetry->pid_p, -2000.0f, 2000.0f),
            mqttSaneOrZero(telemetry->pid_i, -2000.0f, 2000.0f),
            mqttSaneOrZero(telemetry->pid_d, -2000.0f, 2000.0f),
            mqttSaneOrZero(telemetry->pid_output, -2000.0f, 2000.0f));

    if (written <= 0 || written >= (int) sizeof(payload)) {
        LOGGER.error("MQTT STATE payload too large");
        return;
    }

    if (!mqtt.publish(state_topic, payload))
        LOGGER.error(String("MQTT publish failed: ") + state_topic);
}

void MqttController::reloadFromSettings() {
    force_reconnect = true;
    ensureDisconnected();
    last_reconnect_attempt_ms = 0;
    last_state_publish_ms = 0;
    state_topic[0] = 0;
    command_topic[0] = 0;
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

        GlobalSettings *globalSettings = settingsManager ? settingsManager->getSettings() : nullptr;
        if (globalSettings && globalSettings->mqttPublish) {
            unsigned long publishIntervalMs = 6000;
            int configuredIntervalMs = globalSettings->send_data_to_mqtt_interval_ms;
            if (configuredIntervalMs >= 3000)
                publishIntervalMs = (unsigned long) configuredIntervalMs;

            unsigned long nowMs = millis();
            if (last_state_publish_ms == 0 || (nowMs - last_state_publish_ms) >= publishIntervalMs) {
                last_state_publish_ms = nowMs;
                publishState();
            }
        }
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
