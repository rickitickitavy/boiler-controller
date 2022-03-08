#ifndef EFLAME328_GLOBALSETTINGS_H
#define EFLAME328_GLOBALSETTINGS_H

#define GLOBAL_CURRENT_SETTINGS_VERSION 1
#define GLOBAL_SETTINGS_MARKER_0 0x31
#define GLOBAL_SETTINGS_MARKER_1 0x32
#define GLOBAL_SETTINGS_MARKER_2 0x33
#define GLOBAL_SETTINGS_MARKER_3 0x31

#define MAX_SENSORS_COUNT 20
#define SENSORS_ADDR_SIZE 8

struct NetworkSettings {
    /**
  * Настройки WiFi
  */
    char ssid[64];
    char password[64];

    /**
     * Имя станции светильника в сети
     */
    char hostName[64];
};

struct GlobalSettings {
    /**
     * Признак, что настройки записаны, а не пустое пространство. Маркеом является определенныя последовательность
    */
    char initMarker[4];

    /**
     * Версия настроек
    */
    unsigned char version;

    /**
     * Сетевые настройки и настройки WiFi
     */
    NetworkSettings network;

    /**
     * Mqtt server or IP
     */
    char mqttServer[64];

    /**
     * Mqtt port. defaul 1883
     */
    int mqttPort;

    /**
     * Reconnect interval for MQTT if it broke
     */
    long mqttReconnectIntervalMs;

    /**
     * Unique, in home, deviceName
     */
    char mqttDeviceName[32];

    /**
     * topic for reporting state
     */
    char deviceStateOutgoingTopicPrefix[32];

    /**
     *
     */
    char deviceIHaveBornTopic[64];

    /**
     *
     */
    char deviceIncomingCommandTopicPrefix[32];

    /**
     *
     */
    char mqttServerBornTopic[64];

    /**
     *
     */
    char mqttInputToolTopic[32];

    /**
     *
     */
    char mqttOutputToolTopic[32];

    /**
     * ON or OFF must be
     */
    bool defaultSwitcherState;

    /**
     * value for DIMMER device (1024 is MAX)
     */
    int dimmerPwmValue;

    int scan_sensors_integrval_ms;

    /**
     * sensor addresses
     */
    char ds18D20Addresses[SENSORS_ADDR_SIZE * MAX_SENSORS_COUNT];


};

#endif //EFLAME328_SETTINGS_H

