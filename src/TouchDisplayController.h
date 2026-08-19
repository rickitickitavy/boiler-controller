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

#define COLOR_BACKGROUND ILI9488_WHITE
#define COLOR_CORE SCREEN_COLOR_LIGHT_GRAY
#define COLOR_ACCUMULATOR SCREEN_COLOR_LIGHT_LIGHT_BLUE
#define COLOR_CONTROLLER SCREEN_LIGHT_LIGHT_GREEN

#define COLOR_ACCUMULATED SCREEN_LIGHT_ORANGE

#define UI_PAGE_0_COLOR_MAIN_BACKGROUND ILI9488_BLACK
#define UI_PAGE_0_COLOR_GAUGE_BACKGROUND_NORMAL 0x444444
#define UI_PAGE_0_COLOR_GAUGE_BACKGROUND_ERROR 0x67000D
#define UI_PAGE_0_COLOR_GAUGE_BACKGROUND_OPENED 0xFF4444
#define UI_PAGE_0_COLOR_GAUGE_INITFIRE_EST_MANY 0x44FF44
#define UI_PAGE_0_COLOR_GAUGE_INITFIRE_EST_LOW 0xBCBC00
#define UI_PAGE_0_COLOR_GAUGE_INITFIRE_EST_VERY_LOW 0xFF4444
#define UI_PAGE_0_COLOR_GAUGE_INITFIRE_SPENT 0x222222
#define UI_PAGE_0_SIZE_GAUGE_WIDTH 123
#define UI_PAGE_0_SIZE_GAUGE_HEIGHT 100

#define UI_COLOR_GAUGE_GREEN 0x009815
#define UI_COLOR_GAUGE_BLUE 0x0000ff
#define UI_COLOR_GAUGE_YELLOW 0xFFB32F
#define UI_COLOR_GAUGE_RED 0x00ff0000

#define BUTTON_DELAY_ANTI_BUZZLE 150
#define BUTTON_DETECT_X_Y_COUNT 50
#define BUTTONS_MAX_COUNT 10

#define SCREEN_INDEX_MAIN 0
#define SCREEN_INDEX_SETTINGS 1

#define UI_SETTINGS_IDLE_MS 20000
#define UI_SETTINGS_INFO_REFRESH_MS 1000
#define UI_SETTINGS_TAB_COUNT 8
#define UI_SETTINGS_SCREEN_WIDTH 480
#define UI_SETTINGS_SCREEN_HEIGHT 320
#define UI_SETTINGS_TAB_WIDTH ((UI_SETTINGS_SCREEN_WIDTH / 8) + 32)
#define UI_SETTINGS_TAB_MARGIN 6
#define UI_SETTINGS_TAB_GAP 4
#define UI_SETTINGS_TAB_RADIUS 10
#define UI_SETTINGS_CONTENT_RADIUS 8
#define UI_SETTINGS_CONTENT_PAD 16
#define UI_SETTINGS_INFO_LINE_GAP 28
#define UI_SETTINGS_INFO_BAR_HEIGHT 20
#define UI_SETTINGS_INFO_BAR_BASELINE_OFFSET 16
#define UI_SETTINGS_FOOTER_HEIGHT 26

// little grayed white page / unselected tab fill
#define UI_SETTINGS_COLOR_PAGE 0xEF7D
#define UI_SETTINGS_COLOR_TAB_SELECTED 0x2A79
#define UI_SETTINGS_COLOR_TAB_BORDER 0xC618
#define UI_SETTINGS_COLOR_TAB_TEXT_SELECTED 0xFFFF
#define UI_SETTINGS_COLOR_TAB_TEXT_UNSELECTED 0x0000
#define UI_SETTINGS_COLOR_GUTTER 0xE71C

#include <ILI9488.h>
#include <Adafruit_GFX.h>
#include <lib/ui/button/Button.h>
#include "Telemetry.h"
#include "lib/ui/gauge/Gauge.h"
#include "HeaterController.h"
#include <lib/xpt2046/xpt2046.h>

class SettingsManager;
class SettingsTftForms;
class SensorController;
class WiFiController;
class MqttController;


class TouchDisplayController {
protected:
    static TouchDisplayController *instance;
    HeaterController *heaterController;
    SettingsManager *settingsManager;
    SettingsTftForms *settingsForms;
    MqttController *mqttController;

    ILI9488 *tft;
    XPT2046 *touch;

    uint16_t _touch_x, _touch_y;

    int screen_index = -1;
    int settings_tab_index = 0;
    unsigned long settings_last_activity_ms = 0;
    unsigned long settings_info_last_refresh_ms = 0;
    char settings_info_mac_drawn[48];
    char settings_info_status_drawn[64];
    char settings_info_mqtt_drawn[48];
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
    Gauge *gauge_accumulated_energy;
    Gauge *gauge_acc_bottom_tempr;

    Button *button_pumps;
    Button *button_init_fire;
    Button *button_settings;

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
    void drawScreen1(TelemetryDataRecord *telemetryDataRecord);
    void drawPumpState(int index, bool is_on);
    void drawGauges();

    void initScreen0();
    void initScreenSettings();
    void drawScreenSettings();
    void drawSettingsTab(int tab_index);
    void drawSettingsContentChrome();
    void drawSettingsJoinSeam();
    void drawSettingsContent();
    void drawSettingsInfoPage(bool force);
    void drawSettingsInfoBar(int16_t text_x, int16_t baseline_y, int16_t bar_w, const char *text);
    void getSettingsContentRect(int16_t &x, int16_t &y, int16_t &w, int16_t &h);
    void getSettingsTabSlotRect(int tab_index, int16_t &x, int16_t &y, int16_t &w, int16_t &h);
    void getSettingsTabRect(int tab_index, int16_t &x, int16_t &y, int16_t &w, int16_t &h);
    int hitTestSettingsTab(uint16_t x, uint16_t y);
    void noteSettingsActivity();
    void handleSettingsTouch(DisplayButtonEvent event);
    void drawField(const char *msg, int txt_x, int txt_y, int width, int font_color, int bg_color);
    void drawIntField(const char *msg, int value, int txt_x, int txt_y, int width, int font_color, int bg_color);
    void drawFloatField(const char *msg, float value, int txt_x, int txt_y, int width, int font_color, int bg_color);

    static void initFireButtonAction(DisplayButtonEvent event);
    static void initFireButtonDrawAction(DisplayBuffer *canvas);
    static void settingsButtonAction(DisplayButtonEvent event);
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
    void setSettingsManager(SettingsManager *settingsManager);
    void setSensorController(SensorController *sensorController);
    void setWiFiController(WiFiController *wiFiController);
    void setMqttController(MqttController *mqttController);

    void dirty();

    bool isDirty();

    void setTimeToLiveValue(long value);
};


#endif //BASE_ESP8266_MQTT_TouchDisplayController_H
