#ifndef BOILERCONTROLLER_SETTINGSTFTFORMS_H
#define BOILERCONTROLLER_SETTINGSTFTFORMS_H

#include <ILI9488.h>
#include <functional>
#include <lib/ui/form/LabeledTextField.h>
#include <lib/ui/form/LabeledNumberField.h>
#include <lib/ui/form/LabeledDropDown.h>
#include <lib/ui/form/Label.h>
#include <lib/ui/keyboard/OnScreenKeyboard.h>

class SettingsManager;
class SensorController;
class WiFiController;
class MqttController;

#define SETTINGS_FOOTER_HEIGHT 26
#define SETTINGS_SENSOR_FIELDS 10
#define SETTINGS_SERVO_FIELDS 15
#define SETTINGS_MQTT_TEXT 8
#define SETTINGS_CAPACITY_FIELDS 4
#define SETTINGS_CAPACITY_HEADERS 1
#define SETTINGS_CAPACITY_SLOTS (SETTINGS_CAPACITY_HEADERS + SETTINGS_CAPACITY_FIELDS)
#define SETTINGS_RULES_FIELDS 15
#define SETTINGS_RULES_HEADERS 3
#define SETTINGS_RULES_SLOTS (SETTINGS_RULES_HEADERS + SETTINGS_RULES_FIELDS)
#define SETTINGS_MODES_FIELDS 18
#define SETTINGS_MODES_HEADERS 4
#define SETTINGS_MODES_SLOTS (SETTINGS_MODES_HEADERS + SETTINGS_MODES_FIELDS)
#define SETTINGS_SCROLLBAR_WIDTH 10
#define SETTINGS_SCROLLBAR_MARGIN 3
#define SETTINGS_SCROLLBAR_MIN_THUMB 24
#define SETTINGS_DROPDOWN_VISIBLE_ROWS 5
#define SETTINGS_DROPDOWN_BOTTOM_PAD (FORM_DROPDOWN_ROW_HEIGHT * SETTINGS_DROPDOWN_VISIBLE_ROWS + 8)

#define UI_SETTINGS_TAB_INFO 0
#define UI_SETTINGS_TAB_WIFI 1
#define UI_SETTINGS_TAB_MQTT 2
#define UI_SETTINGS_TAB_SENSORS 3
#define UI_SETTINGS_TAB_SERVO 4
#define UI_SETTINGS_TAB_CAPACITIES 5
#define UI_SETTINGS_TAB_RULES 6
#define UI_SETTINGS_TAB_MODES 7

#define SETTINGS_EDIT_NUM_CAPACITY 200
#define SETTINGS_EDIT_NUM_RULES 300
#define SETTINGS_EDIT_NUM_MODES 400

class SettingsTftForms {
private:
    ILI9488 *tft;
    SettingsManager *settingsManager;
    SensorController *sensorController;
    WiFiController *wiFiController;
    MqttController *mqttController;
    OnScreenKeyboard *keyboard;

    LabeledTextField *wifi_ssid;
    LabeledTextField *wifi_password;

    LabeledTextField *mqtt_text[SETTINGS_MQTT_TEXT];
    LabeledNumberField *mqtt_port;
    LabeledNumberField *mqtt_reconnect;

    LabeledDropDown *sensor_fields[SETTINGS_SENSOR_FIELDS];
    LabeledNumberField *servo_fields[SETTINGS_SERVO_FIELDS];

    Label *capacity_headers[SETTINGS_CAPACITY_HEADERS];
    LabeledNumberField *capacity_fields[SETTINGS_CAPACITY_FIELDS];

    Label *rules_headers[SETTINGS_RULES_HEADERS];
    LabeledNumberField *rules_fields[SETTINGS_RULES_FIELDS];

    Label *modes_headers[SETTINGS_MODES_HEADERS];
    LabeledNumberField *modes_fields[SETTINGS_MODES_FIELDS];

    int scroll_y;
    int content_x, content_y, content_w, content_h;
    int active_tab;
    int editing_kind;
    int editing_index;
    char edit_backup[FORM_MAX_VALUE_LEN];
    bool cancel_requested;
    std::function<void()> chrome_redraw;

    void layoutFields();
    void closeOverlays(bool redraw_footer);
    void openKeyboardForText(LabeledTextField *field, int max_len);
    void openKeyboardForNumber(LabeledNumberField *field);
    void placeAndOpenKeyboard(const char *initial, int max_len, KeyboardMode mode, int field_y, int field_h);
    void applyKeyboardText(const char *text);
    void redrawEditingField();
    void dismissKeyboard();
    void restoreUiChrome();
    int fieldTop(int index_in_tab) const;
    int tabContentHeight() const;
    int maxScroll() const;
    bool needsScrollbar() const;
    void getScrollbarTrack(int16_t &x, int16_t &y, int16_t &w, int16_t &h) const;
    void getScrollbarThumb(int16_t &x, int16_t &y, int16_t &w, int16_t &h) const;
    void drawScrollbar();
    void setScrollY(int value);
    bool handleScrollbarTouch(uint16_t tx, uint16_t ty);
    void redrawScrolledBody();
    void openSensorDropdown(int index);
    int footerTop() const;
    void showApplyingMessageBox();
    void applyNumberLimits(LabeledNumberField *field, const char *param_name, const char *base_label);
    int textMaxLenForParam(const char *param_name, int fallback) const;
    void applyLimitsFromNavigator();
    void styleSectionHeader(Label *header);
    void placeSectionHeader(Label *header, int fieldX, int fieldWidth, int slotIndex);
    LabeledNumberField *editingNumberField();

public:
    SettingsTftForms(ILI9488 *tft);
    ~SettingsTftForms();

    void setSettingsManager(SettingsManager *sm);
    void setSensorController(SensorController *sc);
    void setWiFiController(WiFiController *wc);
    void setMqttController(MqttController *controller);
    void setChromeRedraw(std::function<void()> fn);

    void setContentRect(int x, int y, int w, int h);
    void setActiveTab(int tab_index);
    void loadDraftsFromSettings();
    void reload();
    bool save();
    void discardAndCloseOverlays();
    bool consumeCancelRequest();

    void drawFooter();
    void drawTabBody();
    bool handleTouch(uint16_t x, uint16_t y);
    bool isOverlayOpen() const;
    void drawOverlays();
};

#endif
