#ifndef EFLAME328_GLOBALSETTINGS_H
#define EFLAME328_GLOBALSETTINGS_H

#define GLOBAL_CURRENT_SETTINGS_VERSION 1
#define GLOBAL_SETTINGS_MARKER_0 0x32
#define GLOBAL_SETTINGS_MARKER_1 0x32
#define GLOBAL_SETTINGS_MARKER_2 0x33
#define GLOBAL_SETTINGS_MARKER_3 0x37

#define MAX_PROFILE_COUNTER 10
#define SYSTEM_PROFILE_COUNTER 5


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
};

#endif //EFLAME328_SETTINGS_H

