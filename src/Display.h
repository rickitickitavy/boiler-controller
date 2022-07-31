//
// Created by dsporykhin on 24.07.22.
//

#ifndef BASE_ESP8266_MQTT_DISPLAY_H
#define BASE_ESP8266_MQTT_DISPLAY_H

#define FIRST_COLUMN_WIDTH 230
#define SCREEN_0_DATA_LENGTH 10
#define SCREEN_0_MAX_NAME_LENGTH 16
#define SCREEN_COLOR_LIGHT_GRAY 0xCE79

#include "lib/adafruit/Adafruit_GFX.h"
#include "lib/adafruit/Adafruit_ST7789.h"
#include "lib/adafruit/Fonts/FreeMonoBoldOblique18pt7b.h"
#include "Telemetry.h"

struct NameValue{
    char name[SCREEN_0_MAX_NAME_LENGTH + 1];
    char value[7];
};

class Display {
private:
    Adafruit_ST7789 *tft;

    int screen_index = -1;
    NameValue screen_0_data[SCREEN_0_DATA_LENGTH];

    void drawScreen0();

public:
    Display();
    void printStatus(const char *status);

    void setScreenIndex(int index);
    void drawScreen();

    void setScreen0Parameter(int index, const char *name, const char *value);

    void updateInfo(TelemetryDataRecord *telemetryDataRecord);
};


#endif //BASE_ESP8266_MQTT_DISPLAY_H
