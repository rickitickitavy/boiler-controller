#ifndef EFLAME328_GLOBALSETTINGS_H
#define EFLAME328_GLOBALSETTINGS_H

#define GLOBAL_CURRENT_SETTINGS_VERSION 1
#define GLOBAL_SETTINGS_MARKER_0 0x32
#define GLOBAL_SETTINGS_MARKER_1 0x32
#define GLOBAL_SETTINGS_MARKER_2 0x33
#define GLOBAL_SETTINGS_MARKER_3 0x34

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

    /**
     * Резерв сюда будут идти вставки новых настроек, чтобы не сбить имеющиеся настройки при расширении
     */
    char reserved[64];

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
};

#endif //EFLAME328_SETTINGS_H

