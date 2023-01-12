//
// Created by dsporykhin on 24.07.22.
//

#ifndef BASE_ESP8266_MQTT_DISPLAY_H
#define BASE_ESP8266_MQTT_DISPLAY_H

#define SCREEN_COLOR_GRAY 0x8410
#define SCREEN_COLOR_LIGHT_GRAY 0xCE79
#define SCREEN_COLOR_MID_LIGHT_GRAY 0x4208

#define SCREEN_COLOR_CYAN 0x07FF

#define SCREEN_COLOR_ORANGE 0xFD64

#define SCREEN_LIGHT_BLUE 0x94BD
#define SCREEN_COLOR_LIGHT_LIGHT_BLUE 0xBE7F

#define SCREEN_COLOR_LIGHT_RED 0xFBEF
#define SCREEN_COLOR_LIGHT_LIGHT_RED 0xFDB5

//#define SCREEN_GREEN 0x0FE0
#define SCREEN_ORANGE 0xFBC4
#define SCREEN_LIGHT_ORANGE 0xFED9
#define SCREEN_GREEN 0x0600
#define SCREEN_LIGHT_LIGHT_GREEN 0xDFF9

#define COLOR_BACKGROUND ST77XX_WHITE
#define COLOR_CORE SCREEN_COLOR_LIGHT_GRAY
#define COLOR_CONTOUR_ACCUMULATOR ST77XX_BLUE
#define COLOR_ACCUMULATOR SCREEN_COLOR_LIGHT_LIGHT_BLUE
#define COLOR_CONTROLLER SCREEN_LIGHT_LIGHT_GREEN
#define COLOR_CONTOUR_CONTROLLER SCREEN_GREEN

#define COLOR_ACCUMULATED SCREEN_LIGHT_ORANGE
#define COLOR_CONTOUR_ACCUMULATED SCREEN_ORANGE

#define COLOR_RADIATOR SCREEN_GREEN


//1111 1    111 111    1 1111
//1111 1    011 111    0 1111
//0000 0    110 000    0 1001

// 217 255 203
#include <lib/adafruit/ILI9488.h>
#include "lib/adafruit/Adafruit_GFX.h"
#include "lib/adafruit/Adafruit_ST7789.h"
#include "lib/adafruit/Fonts/FreeMonoBoldOblique18pt7b.h"
#include "Telemetry.h"

class Display {
private:
    ILI9488 *tft;

    int screen_index = -1;
    TelemetryDataRecord savedDataRecord;
    bool telemetry_initialized;

    void drawScreen0(TelemetryDataRecord *telemetryDataRecord);
    void drawPumpState(int index, bool is_on);

    void initScreen0();
    void drawField(const char *msg, int txt_x, int txt_y, int width, int font_color, int bg_color);
    void drawIntField(const char *msg, int value, int txt_x, int txt_y, int width, int font_color, int bg_color);
    void drawFloatField(const char *msg, float value, int txt_x, int txt_y, int width, int font_color, int bg_color);

public:
    Display();
    void printStatus(const char *status);

    void setScreenIndex(int index);
    void drawScreen();

    void updateInfo(TelemetryDataRecord *telemetryDataRecord);

    ILI9488 *getTft();
};


#endif //BASE_ESP8266_MQTT_DISPLAY_H
