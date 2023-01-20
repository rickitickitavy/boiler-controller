//
// Created by dsporykhin on 24.07.22.
//

#ifndef BASE_ESP8266_MQTT_TouchDisplayController_H
#define BASE_ESP8266_MQTT_TouchDisplayController_H

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
#define COLOR_ACCUMULATOR SCREEN_COLOR_LIGHT_LIGHT_BLUE
#define COLOR_CONTROLLER SCREEN_LIGHT_LIGHT_GREEN

#define COLOR_ACCUMULATED SCREEN_LIGHT_ORANGE

#define UI_PAGE_0_COLOR_MAIN_BACKGROUND ILI9488_BLACK
#define UI_PAGE_0_COLOR_GAUGE_BACKGROUND 0x444444
#define UI_PAGE_0_SIZE_GAUGE_WIDTH 123
#define UI_PAGE_0_SIZE_GAUGE_HEIGHT 100

#define UI_COLOR_GAUGE_GREEN 0x009815
#define UI_COLOR_GAUGE_BLUE 0x0000ff
#define UI_COLOR_GAUGE_YELLOW 0xFFB32F
#define UI_COLOR_GAUGE_RED 0x00ff0000

#define BUTTON_DELAY_ANTI_BUZZLE 150
#define BUTTON_DETECT_X_Y_COUNT 50
#define BUTTONS_MAX_COUNT 10

//1111 1    111 111    1 1111
//1111 1    011 111    0 1111
//0000 0    110 000    0 1001

// 217 255 203
#include <lib/adafruit/ILI9488.h>
#include <lib/ui/button/Button.h>
#include "lib/adafruit/Adafruit_GFX.h"
#include "lib/adafruit/Adafruit_ST7789.h"
#include "lib/adafruit/Fonts/FreeMonoBoldOblique18pt7b.h"
#include "Telemetry.h"
#include "lib/ui/gauge/Gauge.h"
#include "HeaterController.h"
#include <lib/xpt2046/xpt2046.h>


class TouchDisplayController {
protected:
    static TouchDisplayController *instance;
    HeaterController *heaterController;

    ILI9488 *tft;
    XPT2046 *touch;

    uint16_t _touch_x, _touch_y;

    int screen_index = -1;
    TelemetryDataRecord savedDataRecord;
    bool telemetry_initialized;

    // row 1
    Gauge *gauge_core_tempr;
    Gauge *gauge_core_power;
    Gauge *gauge_warm_flow_tempr;

    // row 2
    Gauge *gauge_core_output_tempr;
    Gauge *gauge_power_balance;
    Gauge *gauge_acc_top_tempr;

    // row 3
    Gauge *gauge_core_input_tempr;
    Gauge *gauge_power;
    Gauge *gauge_acc_bottom_tempr;

    Button *button_pumps;
    Button *button_init_fire;

    DisplayBuffer *defaultDisplayBuffer;

    bool _need_redraw;

    long last_touch_state_changed_at;
    bool screen_touched;

    Button *_buttons[BUTTONS_MAX_COUNT];
    int _buttons_count;

    void handleTouchAction(DisplayButtonEvent event);
    bool read_x_y(uint16_t &x, uint16_t &y);

    uint8_t *loadImage(const char *file_name, uint16_t width, uint16_t height);

    void drawScreen0(TelemetryDataRecord *telemetryDataRecord);
    void drawPumpState(int index, bool is_on);
    void drawGauges();

    void initScreen0();
    void drawField(const char *msg, int txt_x, int txt_y, int width, int font_color, int bg_color);
    void drawIntField(const char *msg, int value, int txt_x, int txt_y, int width, int font_color, int bg_color);
    void drawFloatField(const char *msg, float value, int txt_x, int txt_y, int width, int font_color, int bg_color);

    static void initFireButtonAction(DisplayButtonEvent event);
    void addButton(Button *button);

public:
    TouchDisplayController();
    void handle();

    void printStatus(const char *status);

    void setScreenIndex(int index);
    void drawScreen();

    void updateInfo(TelemetryDataRecord *telemetryDataRecord);

    ILI9488 *getTft();

    void setHeaterController(HeaterController *heaterController);

    void dirty();

    bool isDirty();
};


#endif //BASE_ESP8266_MQTT_TouchDisplayController_H
