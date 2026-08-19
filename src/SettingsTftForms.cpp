#include <Fonts/FreeSans9pt7b.h>
#include <Fonts/FreeSans12pt7b.h>
#include <string.h>
#include <stdio.h>
#include "SettingsTftForms.h"
#include "SettingsManager.h"
#include "SettingsNavigator.h"
#include "ParamDescriptor.h"
#include "SensorController.h"
#include "WiFiController.h"
#include "MqttController.h"
#include "Defines.h"
#include "Converter.h"
#include "HeaterSettings.h"
#include <lib/ui/form/FormColors.h>

#define UI_SETTINGS_SCREEN_WIDTH 480
#define UI_SETTINGS_SCREEN_HEIGHT 320
#define UI_SETTINGS_COLOR_PAGE 0xEF7D
#define UI_SETTINGS_COLOR_GUTTER 0xE71C
#define UI_SETTINGS_CONTENT_RADIUS 8
#define UI_SETTINGS_SECTION_HEADER_FG 0x2A79

static const int SENSOR_SETTING_INDEX[SETTINGS_SENSOR_FIELDS] = {
        T_SENS_INDEX_INTERNAL,
        T_SENS_INDEX_CORE,
        T_SENS_INDEX_OUTPUT_FLOW,
        T_SENS_INDEX_INPUT_FLOW,
        T_SENS_INDEX_ACC_TOP,
        T_SENS_INDEX_ACC_MID_HI,
        T_SENS_INDEX_ACC_MID_LO,
        T_SENS_INDEX_ACC_BOTTOM,
        T_SENS_INDEX_FORWARD_FLOW,
        T_SENS_INDEX_BACKWARD_FLOW
};

static const char *SENSOR_LABELS_FIXED[SETTINGS_SENSOR_FIELDS] = {
        "Internal", "Heater core", "Output flow", "Input flow",
        "Acc top", "Acc mid hi", "Acc mid lo", "Acc bottom",
        "Forward flow", "Backward flow"
};

struct NumberFieldSpec {
    const char *label;
    const char *param;
    bool is_float;
};

static const NumberFieldSpec CAPACITY_SPECS[SETTINGS_CAPACITY_FIELDS] = {
        {"Heater core L", "heater>capacities>heater_core_ltr", false},
        {"Accumulator L", "heater>capacities>accumulator_ltr", false},
        {"Boiler L", "heater>capacities>boiler_ltr", false},
        {"Pipes/radiators L", "heater>capacities>pipes_and_radiators_ltr", false},
};

static const NumberFieldSpec RULES_SPECS[SETTINGS_RULES_FIELDS] = {
        {"Scan interval ms", "heater>scan_interval_ms", false},
        {"Pwr to refuel W", "heater>temperature>core_pwr_to_refuel", true},
        {"Core target C", "heater>temperature>core_target", true},
        {"Core overheat C", "heater>temperature>core_overheat", true},
        {"Core critical C", "heater>temperature>core_critical", true},
        {"Acc target C", "heater>temperature>term_accumulator_target", true},
        {"SMA period sec", "heater>temperature>tempr_sma_period_sec", false},
        {"Core power EMA", "heater>temperature>core_power_ema", false},
        {"Core dP EMA", "heater>temperature>core_d_power_ema", false},
        {"Core dT EMA", "heater>temperature>core_d_tempr_ema", false},
        {"Start pumps on C", "heater>cool_pumps>start_on_tempr", true},
        {"Start dT core-in", "heater>cool_pumps>start_on_d_btw_core_inp", true},
        {"Stop dT from start", "heater>cool_pumps>stop_on_d_btw_start_core", true},
        {"ICPD interval sec", "heater>interval_for_calc_power_acc_sec", false},
        {"ICPD close pwr W", "heater>core_power_to_close_upper_door_W", false},
};

static const NumberFieldSpec MODES_SPECS[SETTINGS_MODES_FIELDS] = {
        {"Warm on power W", "heater>stanby>start_warm_on_power", true},
        {"Pwr to PID W", "heater>warming>pwr_to_sw_to_PID", true},
        {"Core T to PID C", "heater>warming>core_tempr_to_sw", true},
        {"Time to PID sec", "heater>warming>time_to_reach_pwr_sec", false},
        {"Smoke door %", "heater>warming>smoke_door_value_prc", false},
        {"Upper door %", "heater>warming>upper_door_value_prc", false},
        {"Smoke door %", "heater>pid>smoke_door_value_prc", false},
        {"Upper door %", "heater>pid>upper_door_value_prc", false},
        {"P coef", "heater>pid>p", true},
        {"I coef", "heater>pid>i", true},
        {"D coef", "heater>pid>d", true},
        {"Max I", "heater>pid>max_i", true},
        {"Min I", "heater>pid>min_i", true},
        {"Pwr to warm W", "heater>pid>pwr_to_sw_to_warm", true},
        {"Oxy door to warm", "heater>pid>oxy_door_to_sw_warm", true},
        {"T to standby C", "heater>final_cool>tempr_to_sw_to_stby", true},
        {"Pwr to warm W", "heater>final_cool>start_on_d_btw_core_inp", true},
        {"Delay to standby s", "heater>final_cool>delay_to_sw_to_stnby", false},
};

// Slot layout: header indices then field indices interleaved by section.
// Capacities: H0, F0..F3
// Rules: H0, F0..F9, H1, F10..F12, H2, F13..F14
// Modes: H0, F0, H1, F1..F5, H2, F6..F14, H3, F15..F17

static int capacitySlotForField(int field_i) { return 1 + field_i; }

static int rulesSlotForField(int field_i) {
    if (field_i < 10) return 1 + field_i;
    if (field_i < 13) return 12 + (field_i - 10);
    return 16 + (field_i - 13);
}

static int modesSlotForField(int field_i) {
    if (field_i < 1) return 1 + field_i;
    if (field_i < 6) return 3 + (field_i - 1);
    if (field_i < 15) return 9 + (field_i - 6);
    return 19 + (field_i - 15);
}

static int rulesHeaderSlot(int header_i) {
    if (header_i == 0) return 0;
    if (header_i == 1) return 11;
    return 15;
}

static int modesHeaderSlot(int header_i) {
    if (header_i == 0) return 0;
    if (header_i == 1) return 2;
    if (header_i == 2) return 8;
    return 18;
}

SettingsTftForms::SettingsTftForms(ILI9488 *tft)
        : tft(tft), settingsManager(nullptr), sensorController(nullptr), wiFiController(nullptr),
          mqttController(nullptr), keyboard(nullptr), scroll_y(0), content_x(0), content_y(0),
          content_w(100), content_h(100), active_tab(1), editing_kind(0), editing_index(-1),
          cancel_requested(false) {
    edit_backup[0] = 0;
    keyboard = new OnScreenKeyboard(tft);

    wifi_ssid = new LabeledTextField(tft, 0, 0, 100, "WiFi SSID");
    wifi_password = new LabeledTextField(tft, 0, 0, 100, "WiFi password");

    mqtt_text[0] = new LabeledTextField(tft, 0, 0, 100, "MQTT server");
    mqtt_port = new LabeledNumberField(tft, 0, 0, 100, "MQTT port", 0, 1, false);
    mqtt_reconnect = new LabeledNumberField(tft, 0, 0, 100, "Reconnect ms", 0, 1, false);
    mqtt_text[1] = new LabeledTextField(tft, 0, 0, 100, "Device name");
    mqtt_text[2] = new LabeledTextField(tft, 0, 0, 100, "Server born topic");
    mqtt_text[3] = new LabeledTextField(tft, 0, 0, 100, "State out topic prefix");
    mqtt_text[4] = new LabeledTextField(tft, 0, 0, 100, "I-have-born topic");
    mqtt_text[5] = new LabeledTextField(tft, 0, 0, 100, "Incoming cmd topic prefix");
    mqtt_text[6] = new LabeledTextField(tft, 0, 0, 100, "MQTT input tool topic");
    mqtt_text[7] = new LabeledTextField(tft, 0, 0, 100, "MQTT output tool topic");

    for (int i = 0; i < SETTINGS_SENSOR_FIELDS; i++)
        sensor_fields[i] = new LabeledDropDown(tft, 0, 0, 100, SENSOR_LABELS_FIXED[i]);

    const char *servo_doors[3] = {"Smoke", "Oxygen", "Upper"};
    const char *servo_base_lbl[5] = {
            "Min impulse us",
            "Max impulse us",
            "Total degrees",
            "Work min angle",
            "Work max angle"
    };
    bool is_float[5] = {false, false, true, false, false};
    int fieldIndex = 0;
    for (int doorIndex = 0; doorIndex < 3; doorIndex++) {
        for (int servoFieldIndex = 0; servoFieldIndex < 5; servoFieldIndex++) {
            char lbl[64];
            snprintf(lbl, sizeof(lbl), "%s: %s", servo_doors[doorIndex], servo_base_lbl[servoFieldIndex]);
            servo_fields[fieldIndex] = new LabeledNumberField(tft, 0, 0, 100, lbl, 0, 1,
                                                              is_float[servoFieldIndex]);
            fieldIndex++;
        }
    }

    capacity_headers[0] = new Label(tft, 0, 0, 100);
    capacity_headers[0]->setText("Water capacities in liters");
    styleSectionHeader(capacity_headers[0]);
    for (int i = 0; i < SETTINGS_CAPACITY_FIELDS; i++) {
        capacity_fields[i] = new LabeledNumberField(tft, 0, 0, 100, CAPACITY_SPECS[i].label, 0, 1,
                                                    CAPACITY_SPECS[i].is_float);
    }

    static const char *rules_header_titles[SETTINGS_RULES_HEADERS] = {
            "Temperatures",
            "Pumps cooling settings",
            "Upper door control settings"
    };
    for (int i = 0; i < SETTINGS_RULES_HEADERS; i++) {
        rules_headers[i] = new Label(tft, 0, 0, 100);
        rules_headers[i]->setText(rules_header_titles[i]);
        styleSectionHeader(rules_headers[i]);
    }
    for (int i = 0; i < SETTINGS_RULES_FIELDS; i++) {
        rules_fields[i] = new LabeledNumberField(tft, 0, 0, 100, RULES_SPECS[i].label, 0, 1,
                                                 RULES_SPECS[i].is_float);
    }

    static const char *modes_header_titles[SETTINGS_MODES_HEADERS] = {
            "Standby mode settings",
            "Warming mode settings",
            "Burning mode settings (PID)",
            "Final cooling mode settings"
    };
    for (int i = 0; i < SETTINGS_MODES_HEADERS; i++) {
        modes_headers[i] = new Label(tft, 0, 0, 100);
        modes_headers[i]->setText(modes_header_titles[i]);
        styleSectionHeader(modes_headers[i]);
    }
    for (int i = 0; i < SETTINGS_MODES_FIELDS; i++) {
        modes_fields[i] = new LabeledNumberField(tft, 0, 0, 100, MODES_SPECS[i].label, 0, 1,
                                                 MODES_SPECS[i].is_float);
    }

    keyboard->setOnChange([this](const char *text) {
        applyKeyboardText(text);
        redrawEditingField();
    });
}

SettingsTftForms::~SettingsTftForms() {
    delete wifi_ssid;
    delete wifi_password;
    for (int i = 0; i < SETTINGS_MQTT_TEXT; i++) delete mqtt_text[i];
    delete mqtt_port;
    delete mqtt_reconnect;
    for (int i = 0; i < SETTINGS_SENSOR_FIELDS; i++) delete sensor_fields[i];
    for (int i = 0; i < SETTINGS_SERVO_FIELDS; i++) delete servo_fields[i];
    for (int i = 0; i < SETTINGS_CAPACITY_HEADERS; i++) delete capacity_headers[i];
    for (int i = 0; i < SETTINGS_CAPACITY_FIELDS; i++) delete capacity_fields[i];
    for (int i = 0; i < SETTINGS_RULES_HEADERS; i++) delete rules_headers[i];
    for (int i = 0; i < SETTINGS_RULES_FIELDS; i++) delete rules_fields[i];
    for (int i = 0; i < SETTINGS_MODES_HEADERS; i++) delete modes_headers[i];
    for (int i = 0; i < SETTINGS_MODES_FIELDS; i++) delete modes_fields[i];
    delete keyboard;
}

void SettingsTftForms::styleSectionHeader(Label *header) {
    if (!header) return;
    header->setColors(UI_SETTINGS_SECTION_HEADER_FG, UI_SETTINGS_COLOR_PAGE);
}

void SettingsTftForms::placeSectionHeader(Label *header, int fieldX, int fieldWidth, int slotIndex) {
    if (!header) return;
    header->setWidth(fieldWidth);
    header->setPosition(fieldX, fieldTop(slotIndex));
}

void SettingsTftForms::setSettingsManager(SettingsManager *sm) {
    settingsManager = sm;
    applyLimitsFromNavigator();
}
void SettingsTftForms::setSensorController(SensorController *sc) { sensorController = sc; }
void SettingsTftForms::setWiFiController(WiFiController *wc) { wiFiController = wc; }
void SettingsTftForms::setMqttController(MqttController *controller) { mqttController = controller; }
void SettingsTftForms::setChromeRedraw(std::function<void()> fn) { chrome_redraw = fn; }

void SettingsTftForms::applyNumberLimits(LabeledNumberField *field, const char *param_name, const char *base_label) {
    if (!field || !settingsManager || !settingsManager->getNavigator())
        return;
    ParamDescriptor *paramDescriptor = settingsManager->getNavigator()->findParamDescriptor(param_name);
    if (!paramDescriptor)
        return;
    const bool as_float = (paramDescriptor->paramType == FLOAT);
    field->setRange(paramDescriptor->minValue, paramDescriptor->maxValue, as_float);
    char lbl[80];
    if (as_float)
        snprintf(lbl, sizeof(lbl), "%s (%.2f..%.2f)", base_label, paramDescriptor->minValue,
                 paramDescriptor->maxValue);
    else
        snprintf(lbl, sizeof(lbl), "%s (%d..%d)", base_label, (int) paramDescriptor->minValue,
                 (int) paramDescriptor->maxValue);
    field->setLabel(lbl);
}

int SettingsTftForms::textMaxLenForParam(const char *param_name, int fallback) const {
    if (!settingsManager || !settingsManager->getNavigator() || !param_name)
        return fallback;
    ParamDescriptor *paramDescriptor = settingsManager->getNavigator()->findParamDescriptor(param_name);
    if (!paramDescriptor)
        return fallback;
    int max_len = (int) paramDescriptor->maxValue;
    if (max_len < 1)
        return fallback;
    if (max_len > FORM_MAX_VALUE_LEN - 1)
        max_len = FORM_MAX_VALUE_LEN - 1;
    return max_len;
}

void SettingsTftForms::applyLimitsFromNavigator() {
    if (!settingsManager || !settingsManager->getNavigator())
        return;

    applyNumberLimits(mqtt_port, "mqtt>port", "MQTT port");
    applyNumberLimits(mqtt_reconnect, "mqtt>reconnectIntervalMs", "Reconnect ms");

    static const char *door_keys[3] = {"smoke", "oxygen", "upper"};
    static const char *door_titles[3] = {"Smoke", "Oxygen", "Upper"};
    static const char *field_keys[5] = {
            "min_impulse_length_us",
            "max_impulse_length_us",
            "total_degrees",
            "working_min_angle",
            "working_max_angle"
    };
    static const char *field_titles[5] = {
            "Min impulse us",
            "Max impulse us",
            "Total degrees",
            "Work min angle",
            "Work max angle"
    };

    int fieldIndex = 0;
    for (int doorIndex = 0; doorIndex < 3; doorIndex++) {
        for (int servoFieldIndex = 0; servoFieldIndex < 5; servoFieldIndex++) {
            char param[80];
            char base[80];
            snprintf(param, sizeof(param), "heater>servos>%s>%s", door_keys[doorIndex], field_keys[servoFieldIndex]);
            snprintf(base, sizeof(base), "%s: %s", door_titles[doorIndex], field_titles[servoFieldIndex]);
            applyNumberLimits(servo_fields[fieldIndex], param, base);
            fieldIndex++;
        }
    }

    for (int i = 0; i < SETTINGS_CAPACITY_FIELDS; i++)
        applyNumberLimits(capacity_fields[i], CAPACITY_SPECS[i].param, CAPACITY_SPECS[i].label);
    for (int i = 0; i < SETTINGS_RULES_FIELDS; i++)
        applyNumberLimits(rules_fields[i], RULES_SPECS[i].param, RULES_SPECS[i].label);
    for (int i = 0; i < SETTINGS_MODES_FIELDS; i++)
        applyNumberLimits(modes_fields[i], MODES_SPECS[i].param, MODES_SPECS[i].label);
}

LabeledNumberField *SettingsTftForms::editingNumberField() {
    if (editing_kind != 2)
        return nullptr;
    if (editing_index == 0) return mqtt_port;
    if (editing_index == 1) return mqtt_reconnect;
    if (editing_index >= 100 && editing_index < 100 + SETTINGS_SERVO_FIELDS)
        return servo_fields[editing_index - 100];
    if (editing_index >= SETTINGS_EDIT_NUM_CAPACITY
        && editing_index < SETTINGS_EDIT_NUM_CAPACITY + SETTINGS_CAPACITY_FIELDS)
        return capacity_fields[editing_index - SETTINGS_EDIT_NUM_CAPACITY];
    if (editing_index >= SETTINGS_EDIT_NUM_RULES
        && editing_index < SETTINGS_EDIT_NUM_RULES + SETTINGS_RULES_FIELDS)
        return rules_fields[editing_index - SETTINGS_EDIT_NUM_RULES];
    if (editing_index >= SETTINGS_EDIT_NUM_MODES
        && editing_index < SETTINGS_EDIT_NUM_MODES + SETTINGS_MODES_FIELDS)
        return modes_fields[editing_index - SETTINGS_EDIT_NUM_MODES];
    return nullptr;
}

void SettingsTftForms::restoreUiChrome() {
    if (chrome_redraw) {
        chrome_redraw();
        return;
    }
    tft->fillRoundRect(content_x, content_y, content_w, content_h, UI_SETTINGS_CONTENT_RADIUS,
                       UI_SETTINGS_COLOR_PAGE);
    drawTabBody();
    drawFooter();
}

void SettingsTftForms::applyKeyboardText(const char *text) {
    if (!text) text = "";
    if (editing_kind == 1 && editing_index == 0) {
        wifi_ssid->setValue(text);
    } else if (editing_kind == 1 && editing_index == 1) {
        wifi_password->setValue(text);
    } else if (editing_kind == 1 && editing_index >= 10 && editing_index < 10 + SETTINGS_MQTT_TEXT) {
        mqtt_text[editing_index - 10]->setValue(text);
    } else if (editing_kind == 2) {
        LabeledNumberField *field = editingNumberField();
        if (field)
            field->getField().setValue(text);
    }
}

void SettingsTftForms::redrawEditingField() {
    if (editing_kind == 1 && editing_index == 0) {
        wifi_ssid->getField().invalidate();
        wifi_ssid->getField().draw();
    } else if (editing_kind == 1 && editing_index == 1) {
        wifi_password->getField().invalidate();
        wifi_password->getField().draw();
    } else if (editing_kind == 1 && editing_index >= 10 && editing_index < 10 + SETTINGS_MQTT_TEXT) {
        mqtt_text[editing_index - 10]->getField().invalidate();
        mqtt_text[editing_index - 10]->getField().draw();
    } else if (editing_kind == 2) {
        LabeledNumberField *field = editingNumberField();
        if (field) {
            field->getField().invalidate();
            field->getField().draw();
        }
    }
}

void SettingsTftForms::dismissKeyboard() {
    if (!keyboard || !keyboard->isVisible())
        return;

    if (editing_kind == 2) {
        LabeledNumberField *field = editingNumberField();
        if (field)
            field->getField().commitText(keyboard->getText());
    }

    if (editing_kind == 1 && editing_index == 0) wifi_ssid->setFocused(false);
    else if (editing_kind == 1 && editing_index == 1) wifi_password->setFocused(false);
    else if (editing_kind == 1 && editing_index >= 10 && editing_index < 10 + SETTINGS_MQTT_TEXT)
        mqtt_text[editing_index - 10]->setFocused(false);
    else if (editing_kind == 2) {
        LabeledNumberField *field = editingNumberField();
        if (field)
            field->setFocused(false);
    }

    editing_kind = 0;
    editing_index = -1;
    keyboard->close();
    restoreUiChrome();
}

void SettingsTftForms::setContentRect(int x, int y, int w, int h) {
    content_x = x;
    content_y = y;
    content_w = w;
    content_h = h;
    layoutFields();
}

void SettingsTftForms::setActiveTab(int tab_index) {
    if (active_tab != tab_index) {
        closeOverlays(false);
        active_tab = tab_index;
        scroll_y = 0;
    }
    layoutFields();
}

int SettingsTftForms::fieldTop(int index_in_tab) const {
    const int gap = 8;
    const int item_h = FORM_LABEL_HEIGHT + FORM_LABEL_FIELD_GAP + FORM_FIELD_HEIGHT + gap;
    return content_y + 8 + index_in_tab * item_h - scroll_y;
}

int SettingsTftForms::tabContentHeight() const {
    int count = 0;
    if (active_tab == UI_SETTINGS_TAB_WIFI) count = 2;
    else if (active_tab == UI_SETTINGS_TAB_MQTT) count = SETTINGS_MQTT_TEXT + 2;
    else if (active_tab == UI_SETTINGS_TAB_SENSORS) count = SETTINGS_SENSOR_FIELDS;
    else if (active_tab == UI_SETTINGS_TAB_SERVO) count = SETTINGS_SERVO_FIELDS;
    else if (active_tab == UI_SETTINGS_TAB_CAPACITIES) count = SETTINGS_CAPACITY_SLOTS;
    else if (active_tab == UI_SETTINGS_TAB_RULES) count = SETTINGS_RULES_SLOTS;
    else if (active_tab == UI_SETTINGS_TAB_MODES) count = SETTINGS_MODES_SLOTS;
    const int gap = 8;
    const int item_h = FORM_LABEL_HEIGHT + FORM_LABEL_FIELD_GAP + FORM_FIELD_HEIGHT + gap;
    int h = 8 + count * item_h;
    if (active_tab == UI_SETTINGS_TAB_SENSORS)
        h += SETTINGS_DROPDOWN_BOTTOM_PAD;
    else
        h += 8;
    return h;
}

int SettingsTftForms::footerTop() const {
    return UI_SETTINGS_SCREEN_HEIGHT - SETTINGS_FOOTER_HEIGHT;
}

int SettingsTftForms::maxScroll() const {
    int m = tabContentHeight() - content_h;
    return m > 0 ? m : 0;
}

bool SettingsTftForms::needsScrollbar() const {
    return active_tab != UI_SETTINGS_TAB_INFO && maxScroll() > 0;
}

void SettingsTftForms::getScrollbarTrack(int16_t &x, int16_t &y, int16_t &w, int16_t &h) const {
    w = SETTINGS_SCROLLBAR_WIDTH;
    x = (int16_t) (content_x + content_w - SETTINGS_SCROLLBAR_WIDTH - SETTINGS_SCROLLBAR_MARGIN);
    y = (int16_t) (content_y + SETTINGS_SCROLLBAR_MARGIN);
    h = (int16_t) (content_h - SETTINGS_SCROLLBAR_MARGIN * 2);
    if (h < 1) h = 1;
}

void SettingsTftForms::getScrollbarThumb(int16_t &x, int16_t &y, int16_t &w, int16_t &h) const {
    int16_t tx, ty, tw, th;
    getScrollbarTrack(tx, ty, tw, th);
    x = tx;
    w = tw;
    const int total = tabContentHeight();
    int thumb_h = th;
    if (total > 0)
        thumb_h = (int) ((long) th * content_h / total);
    if (thumb_h < SETTINGS_SCROLLBAR_MIN_THUMB) thumb_h = SETTINGS_SCROLLBAR_MIN_THUMB;
    if (thumb_h > th) thumb_h = th;
    h = (int16_t) thumb_h;

    const int ms = maxScroll();
    int thumb_y = ty;
    if (ms > 0 && th > thumb_h)
        thumb_y = ty + (int) ((long) (th - thumb_h) * scroll_y / ms);
    y = (int16_t) thumb_y;
}

void SettingsTftForms::drawScrollbar() {
    if (!needsScrollbar())
        return;

    int16_t tx, ty, tw, th;
    getScrollbarTrack(tx, ty, tw, th);
    tft->fillRoundRect(tx, ty, tw, th, 3, 0xCE79);

    int16_t bx, by, bw, bh;
    getScrollbarThumb(bx, by, bw, bh);
    tft->fillRoundRect(bx, by, bw, bh, 3, 0x8410);
}

void SettingsTftForms::redrawScrolledBody() {
    tft->fillRoundRect(content_x, content_y, content_w, content_h, UI_SETTINGS_CONTENT_RADIUS,
                       UI_SETTINGS_COLOR_PAGE);
    drawTabBody();
}

void SettingsTftForms::setScrollY(int value) {
    int ms = maxScroll();
    if (value < 0) value = 0;
    if (value > ms) value = ms;
    if (value == scroll_y)
        return;
    scroll_y = value;
    redrawScrolledBody();
}

bool SettingsTftForms::handleScrollbarTouch(uint16_t tx, uint16_t ty) {
    if (!needsScrollbar())
        return false;

    int16_t track_x, track_y, track_w, track_h;
    getScrollbarTrack(track_x, track_y, track_w, track_h);
    if (tx < (uint16_t) (track_x - 4) || tx >= (uint16_t) (track_x + track_w + 4)
        || ty < (uint16_t) track_y || ty >= (uint16_t) (track_y + track_h))
        return false;

    int16_t thumb_x, thumb_y, thumb_w, thumb_h;
    getScrollbarThumb(thumb_x, thumb_y, thumb_w, thumb_h);
    const int ms = maxScroll();
    if (ms <= 0 || track_h <= thumb_h) {
        setScrollY(0);
        return true;
    }

    int center = (int) ty;
    int new_scroll = (int) ((long) (center - track_y - thumb_h / 2) * ms / (track_h - thumb_h));
    setScrollY(new_scroll);
    return true;
}

void SettingsTftForms::layoutFields() {
    const int pad = 8;
    int fieldX = content_x + pad;
    int fieldWidth = content_w - pad * 2;
    if (needsScrollbar())
        fieldWidth -= (SETTINGS_SCROLLBAR_WIDTH + SETTINGS_SCROLLBAR_MARGIN);
    if (fieldWidth < 40) fieldWidth = 40;

    auto place = [&](auto *widget, int slotIndex) {
        int fieldY = fieldTop(slotIndex);
        widget->setWidth(fieldWidth);
        widget->setPosition(fieldX, fieldY);
    };

    if (active_tab == UI_SETTINGS_TAB_WIFI) {
        place(wifi_ssid, 0);
        place(wifi_password, 1);
    } else if (active_tab == UI_SETTINGS_TAB_MQTT) {
        place(mqtt_text[0], 0);
        place(mqtt_port, 1);
        place(mqtt_reconnect, 2);
        for (int i = 1; i < SETTINGS_MQTT_TEXT; i++)
            place(mqtt_text[i], i + 2);
    } else if (active_tab == UI_SETTINGS_TAB_SENSORS) {
        for (int i = 0; i < SETTINGS_SENSOR_FIELDS; i++)
            place(sensor_fields[i], i);
    } else if (active_tab == UI_SETTINGS_TAB_SERVO) {
        for (int i = 0; i < SETTINGS_SERVO_FIELDS; i++)
            place(servo_fields[i], i);
    } else if (active_tab == UI_SETTINGS_TAB_CAPACITIES) {
        placeSectionHeader(capacity_headers[0], fieldX, fieldWidth, 0);
        for (int i = 0; i < SETTINGS_CAPACITY_FIELDS; i++)
            place(capacity_fields[i], capacitySlotForField(i));
    } else if (active_tab == UI_SETTINGS_TAB_RULES) {
        for (int i = 0; i < SETTINGS_RULES_HEADERS; i++)
            placeSectionHeader(rules_headers[i], fieldX, fieldWidth, rulesHeaderSlot(i));
        for (int i = 0; i < SETTINGS_RULES_FIELDS; i++)
            place(rules_fields[i], rulesSlotForField(i));
    } else if (active_tab == UI_SETTINGS_TAB_MODES) {
        for (int i = 0; i < SETTINGS_MODES_HEADERS; i++)
            placeSectionHeader(modes_headers[i], fieldX, fieldWidth, modesHeaderSlot(i));
        for (int i = 0; i < SETTINGS_MODES_FIELDS; i++)
            place(modes_fields[i], modesSlotForField(i));
    }
}

void SettingsTftForms::loadDraftsFromSettings() {
    if (!settingsManager) return;
    GlobalSettings *globalSettings = settingsManager->getSettings();
    wifi_ssid->setValue(globalSettings->network.ssid);
    wifi_password->setValue(globalSettings->network.password);

    mqtt_text[0]->setValue(globalSettings->mqttServer);
    mqtt_port->setNumber((float) globalSettings->mqttPort);
    mqtt_reconnect->setNumber((float) globalSettings->mqttReconnectIntervalMs);
    mqtt_text[1]->setValue(globalSettings->mqttDeviceName);
    mqtt_text[2]->setValue(globalSettings->mqttServerBornTopic);
    mqtt_text[3]->setValue(globalSettings->deviceStateOutgoingTopicPrefix);
    mqtt_text[4]->setValue(globalSettings->deviceIHaveBornTopic);
    mqtt_text[5]->setValue(globalSettings->deviceIncomingCommandTopicPrefix);
    mqtt_text[6]->setValue(globalSettings->mqttInputToolTopic);
    mqtt_text[7]->setValue(globalSettings->mqttOutputToolTopic);

    for (int i = 0; i < SETTINGS_SENSOR_FIELDS; i++) {
        DropDownList &dropDown = sensor_fields[i]->getField();
        dropDown.clearOptions();
        dropDown.addOption("0000000000000000");
        if (sensorController) {
            char addressHex[SENSORS_ADDR_SIZE * 2 + 1];
            for (int j = 0; j < sensorController->found_sensors_count && j < FORM_DROPDOWN_MAX_OPTIONS - 1; j++) {
                Converter::bytesToAsciiHex(addressHex,
                                           &sensorController->found_sensors_addr[j * SENSORS_ADDR_SIZE],
                                           SENSORS_ADDR_SIZE);
                dropDown.addOption(addressHex);
            }
        }
        char currentAddressHex[SENSORS_ADDR_SIZE * 2 + 1];
        Converter::bytesToAsciiHex(
                currentAddressHex,
                (uint8_t *) &globalSettings->ds18D20Addresses[SENSOR_SETTING_INDEX[i] * SENSORS_ADDR_SIZE],
                SENSORS_ADDR_SIZE);
        dropDown.setSelectedValue(currentAddressHex);
        dropDown.setOpen(false);
    }

    ServosHardwareSettings *servosHardwareSettings = &globalSettings->heaterSettings.servos_hardware_settings;
    ServoHardwareSettings *doorSettings[3] = {
            &servosHardwareSettings->smoke_servo_settings,
            &servosHardwareSettings->oxygen_servo_settings,
            &servosHardwareSettings->upper_door_servo_settings
    };
    int fieldIndex = 0;
    for (int doorIndex = 0; doorIndex < 3; doorIndex++) {
        servo_fields[fieldIndex++]->setNumber((float) doorSettings[doorIndex]->min_impulse_length_us);
        servo_fields[fieldIndex++]->setNumber((float) doorSettings[doorIndex]->max_impulse_length_us);
        servo_fields[fieldIndex++]->setNumber(doorSettings[doorIndex]->total_degrees);
        servo_fields[fieldIndex++]->setNumber((float) doorSettings[doorIndex]->working_min_angle);
        servo_fields[fieldIndex++]->setNumber((float) doorSettings[doorIndex]->working_max_angle);
    }

    VolumeCapacitiesSetting *capacitiesSetting = &globalSettings->heaterSettings.capacities_setting;
    capacity_fields[0]->setNumber((float) capacitiesSetting->heater_core_ltr);
    capacity_fields[1]->setNumber((float) capacitiesSetting->accumulator_ltr);
    capacity_fields[2]->setNumber((float) capacitiesSetting->boiler_ltr);
    capacity_fields[3]->setNumber((float) capacitiesSetting->pipes_and_radiators_ltr);

    TemperatureSettings *temperatureSettings = &globalSettings->heaterSettings.temperatureSettings;
    rules_fields[0]->setNumber((float) globalSettings->heaterSettings.scan_interval_ms);
    rules_fields[1]->setNumber(temperatureSettings->core_power_when_need_to_refuel);
    rules_fields[2]->setNumber(temperatureSettings->core_target);
    rules_fields[3]->setNumber(temperatureSettings->core_overheat);
    rules_fields[4]->setNumber(temperatureSettings->core_critical);
    rules_fields[5]->setNumber(temperatureSettings->thermal_accumulator_target);
    rules_fields[6]->setNumber((float) temperatureSettings->SMA_temperature_period_sec);
    rules_fields[7]->setNumber((float) temperatureSettings->core_power_EMA);
    rules_fields[8]->setNumber((float) temperatureSettings->core_power_diff_EMA);
    rules_fields[9]->setNumber((float) temperatureSettings->core_temp_diff_EMA);

    CoolingByPumpsSettings *coolingByPumpsSettings = &globalSettings->heaterSettings.coolingByPumpsSettings;
    rules_fields[10]->setNumber(coolingByPumpsSettings->start_pumps_temperature);
    rules_fields[11]->setNumber(coolingByPumpsSettings->min_delta_btw_core_and_input_to_start_pumps);
    rules_fields[12]->setNumber(coolingByPumpsSettings->delta_btw_start_and_core_to_stop_pumps);

    rules_fields[13]->setNumber((float) globalSettings->interval_for_calc_power_different_sec);
    rules_fields[14]->setNumber((float) globalSettings->core_power_to_close_upper_door_if_ICPD_W);

    modes_fields[0]->setNumber(globalSettings->heaterSettings.stadbyCoolingSettings.start_warming_cycle_on_power);

    WarmingSettings *warmingSettings = &globalSettings->heaterSettings.warming_settings;
    modes_fields[1]->setNumber(warmingSettings->target_power_to_switch_to_the_PID_mode);
    modes_fields[2]->setNumber(warmingSettings->start_pid_temperature);
    modes_fields[3]->setNumber((float) warmingSettings->time_to_reach_target_power_sec);
    modes_fields[4]->setNumber((float) warmingSettings->smoke_door_value_prcnt);
    modes_fields[5]->setNumber((float) warmingSettings->upper_door_value_prcnt);

    BurningModeSettings *burningSettings = &globalSettings->heaterSettings.burning_settings;
    modes_fields[6]->setNumber((float) burningSettings->smoke_door_value_prcnt);
    modes_fields[7]->setNumber((float) burningSettings->upper_door_value_prcnt);
    modes_fields[8]->setNumber(burningSettings->p);
    modes_fields[9]->setNumber(burningSettings->i);
    modes_fields[10]->setNumber(burningSettings->d);
    modes_fields[11]->setNumber(burningSettings->max_i);
    modes_fields[12]->setNumber(burningSettings->min_i);
    modes_fields[13]->setNumber(burningSettings->power_to_switch_to_warming_mode);
    modes_fields[14]->setNumber(burningSettings->oxygen_door_val_to_warming_mode);

    FinalCoolingSettings *finalCoolingSettings = &globalSettings->heaterSettings.finalCoolingSettings;
    modes_fields[15]->setNumber(finalCoolingSettings->max_temperature_to_switch_to_standBy);
    modes_fields[16]->setNumber(finalCoolingSettings->power_to_switch_to_warming_mode);
    modes_fields[17]->setNumber((float) finalCoolingSettings->delay_to_switch_to_standby_mode_sec);
}

void SettingsTftForms::reload() {
    closeOverlays(false);
    loadDraftsFromSettings();
    scroll_y = 0;
    layoutFields();
    redrawScrolledBody();
    drawFooter();
}

bool SettingsTftForms::save() {
    if (!settingsManager) return false;

    closeOverlays(false);
    showApplyingMessageBox();

    GlobalSettings *globalSettings = settingsManager->getSettings();

    char previousSsid[sizeof(globalSettings->network.ssid)];
    char previousPassword[sizeof(globalSettings->network.password)];
    strncpy(previousSsid, globalSettings->network.ssid, sizeof(previousSsid) - 1);
    previousSsid[sizeof(previousSsid) - 1] = 0;
    strncpy(previousPassword, globalSettings->network.password, sizeof(previousPassword) - 1);
    previousPassword[sizeof(previousPassword) - 1] = 0;

    strncpy(globalSettings->network.ssid, wifi_ssid->getValue(), sizeof(globalSettings->network.ssid) - 1);
    globalSettings->network.ssid[sizeof(globalSettings->network.ssid) - 1] = 0;
    strncpy(globalSettings->network.password, wifi_password->getValue(),
            sizeof(globalSettings->network.password) - 1);
    globalSettings->network.password[sizeof(globalSettings->network.password) - 1] = 0;

    const bool wifiCredentialsChanged =
            (strcmp(previousSsid, globalSettings->network.ssid) != 0)
            || (strcmp(previousPassword, globalSettings->network.password) != 0);

    strncpy(globalSettings->mqttServer, mqtt_text[0]->getValue(), sizeof(globalSettings->mqttServer) - 1);
    globalSettings->mqttServer[sizeof(globalSettings->mqttServer) - 1] = 0;
    globalSettings->mqttPort = (int) mqtt_port->getNumber();
    globalSettings->mqttReconnectIntervalMs = (long) mqtt_reconnect->getNumber();
    strncpy(globalSettings->mqttDeviceName, mqtt_text[1]->getValue(), sizeof(globalSettings->mqttDeviceName) - 1);
    globalSettings->mqttDeviceName[sizeof(globalSettings->mqttDeviceName) - 1] = 0;
    strncpy(globalSettings->mqttServerBornTopic, mqtt_text[2]->getValue(),
            sizeof(globalSettings->mqttServerBornTopic) - 1);
    globalSettings->mqttServerBornTopic[sizeof(globalSettings->mqttServerBornTopic) - 1] = 0;
    strncpy(globalSettings->deviceStateOutgoingTopicPrefix, mqtt_text[3]->getValue(),
            sizeof(globalSettings->deviceStateOutgoingTopicPrefix) - 1);
    globalSettings->deviceStateOutgoingTopicPrefix[sizeof(globalSettings->deviceStateOutgoingTopicPrefix) - 1] = 0;
    strncpy(globalSettings->deviceIHaveBornTopic, mqtt_text[4]->getValue(),
            sizeof(globalSettings->deviceIHaveBornTopic) - 1);
    globalSettings->deviceIHaveBornTopic[sizeof(globalSettings->deviceIHaveBornTopic) - 1] = 0;
    strncpy(globalSettings->deviceIncomingCommandTopicPrefix, mqtt_text[5]->getValue(),
            sizeof(globalSettings->deviceIncomingCommandTopicPrefix) - 1);
    globalSettings->deviceIncomingCommandTopicPrefix[sizeof(globalSettings->deviceIncomingCommandTopicPrefix) - 1] = 0;
    strncpy(globalSettings->mqttInputToolTopic, mqtt_text[6]->getValue(),
            sizeof(globalSettings->mqttInputToolTopic) - 1);
    globalSettings->mqttInputToolTopic[sizeof(globalSettings->mqttInputToolTopic) - 1] = 0;
    strncpy(globalSettings->mqttOutputToolTopic, mqtt_text[7]->getValue(),
            sizeof(globalSettings->mqttOutputToolTopic) - 1);
    globalSettings->mqttOutputToolTopic[sizeof(globalSettings->mqttOutputToolTopic) - 1] = 0;

    for (int i = 0; i < SETTINGS_SENSOR_FIELDS; i++) {
        const char *addressHex = sensor_fields[i]->getField().getSelectedValue();
        Converter::asciiHexToBytes(
                (uint8_t *) &globalSettings->ds18D20Addresses[SENSOR_SETTING_INDEX[i] * SENSORS_ADDR_SIZE],
                addressHex, SENSORS_ADDR_SIZE);
    }

    ServosHardwareSettings *servosHardwareSettings = &globalSettings->heaterSettings.servos_hardware_settings;
    ServoHardwareSettings *doorSettings[3] = {
            &servosHardwareSettings->smoke_servo_settings,
            &servosHardwareSettings->oxygen_servo_settings,
            &servosHardwareSettings->upper_door_servo_settings
    };
    int fieldIndex = 0;
    for (int doorIndex = 0; doorIndex < 3; doorIndex++) {
        doorSettings[doorIndex]->min_impulse_length_us = (int) servo_fields[fieldIndex++]->getNumber();
        doorSettings[doorIndex]->max_impulse_length_us = (int) servo_fields[fieldIndex++]->getNumber();
        doorSettings[doorIndex]->total_degrees = servo_fields[fieldIndex++]->getNumber();
        doorSettings[doorIndex]->working_min_angle = (int) servo_fields[fieldIndex++]->getNumber();
        doorSettings[doorIndex]->working_max_angle = (int) servo_fields[fieldIndex++]->getNumber();
    }

    VolumeCapacitiesSetting *capacitiesSetting = &globalSettings->heaterSettings.capacities_setting;
    capacitiesSetting->heater_core_ltr = (int) capacity_fields[0]->getNumber();
    capacitiesSetting->accumulator_ltr = (int) capacity_fields[1]->getNumber();
    capacitiesSetting->boiler_ltr = (int) capacity_fields[2]->getNumber();
    capacitiesSetting->pipes_and_radiators_ltr = (int) capacity_fields[3]->getNumber();

    TemperatureSettings *temperatureSettings = &globalSettings->heaterSettings.temperatureSettings;
    globalSettings->heaterSettings.scan_interval_ms = (int) rules_fields[0]->getNumber();
    temperatureSettings->core_power_when_need_to_refuel = rules_fields[1]->getNumber();
    temperatureSettings->core_target = rules_fields[2]->getNumber();
    temperatureSettings->core_overheat = rules_fields[3]->getNumber();
    temperatureSettings->core_critical = rules_fields[4]->getNumber();
    temperatureSettings->thermal_accumulator_target = rules_fields[5]->getNumber();
    temperatureSettings->SMA_temperature_period_sec = (int) rules_fields[6]->getNumber();
    temperatureSettings->core_power_EMA = (int) rules_fields[7]->getNumber();
    temperatureSettings->core_power_diff_EMA = (int) rules_fields[8]->getNumber();
    temperatureSettings->core_temp_diff_EMA = (int) rules_fields[9]->getNumber();

    CoolingByPumpsSettings *coolingByPumpsSettings = &globalSettings->heaterSettings.coolingByPumpsSettings;
    coolingByPumpsSettings->start_pumps_temperature = rules_fields[10]->getNumber();
    coolingByPumpsSettings->min_delta_btw_core_and_input_to_start_pumps = rules_fields[11]->getNumber();
    coolingByPumpsSettings->delta_btw_start_and_core_to_stop_pumps = rules_fields[12]->getNumber();

    globalSettings->interval_for_calc_power_different_sec = (int) rules_fields[13]->getNumber();
    globalSettings->core_power_to_close_upper_door_if_ICPD_W = (int) rules_fields[14]->getNumber();

    globalSettings->heaterSettings.stadbyCoolingSettings.start_warming_cycle_on_power =
            modes_fields[0]->getNumber();

    WarmingSettings *warmingSettings = &globalSettings->heaterSettings.warming_settings;
    warmingSettings->target_power_to_switch_to_the_PID_mode = modes_fields[1]->getNumber();
    warmingSettings->start_pid_temperature = modes_fields[2]->getNumber();
    warmingSettings->time_to_reach_target_power_sec = (int) modes_fields[3]->getNumber();
    warmingSettings->smoke_door_value_prcnt = (int) modes_fields[4]->getNumber();
    warmingSettings->upper_door_value_prcnt = (int) modes_fields[5]->getNumber();

    BurningModeSettings *burningSettings = &globalSettings->heaterSettings.burning_settings;
    burningSettings->smoke_door_value_prcnt = (int) modes_fields[6]->getNumber();
    burningSettings->upper_door_value_prcnt = (int) modes_fields[7]->getNumber();
    burningSettings->p = modes_fields[8]->getNumber();
    burningSettings->i = modes_fields[9]->getNumber();
    burningSettings->d = modes_fields[10]->getNumber();
    burningSettings->max_i = modes_fields[11]->getNumber();
    burningSettings->min_i = modes_fields[12]->getNumber();
    burningSettings->power_to_switch_to_warming_mode = modes_fields[13]->getNumber();
    burningSettings->oxygen_door_val_to_warming_mode = modes_fields[14]->getNumber();

    FinalCoolingSettings *finalCoolingSettings = &globalSettings->heaterSettings.finalCoolingSettings;
    finalCoolingSettings->max_temperature_to_switch_to_standBy = modes_fields[15]->getNumber();
    finalCoolingSettings->power_to_switch_to_warming_mode = modes_fields[16]->getNumber();
    finalCoolingSettings->delay_to_switch_to_standby_mode_sec = (int) modes_fields[17]->getNumber();

    settingsManager->saveSetting(false);
    if (wiFiController && wifiCredentialsChanged)
        wiFiController->reapplyNetworkSettings();
    if (mqttController)
        mqttController->reloadFromSettings();

    restoreUiChrome();
    return true;
}

void SettingsTftForms::showApplyingMessageBox() {
    const int bw = 360;
    const int bh = 88;
    const int bx = (UI_SETTINGS_SCREEN_WIDTH - bw) / 2;
    const int by = (UI_SETTINGS_SCREEN_HEIGHT - bh) / 2;
    tft->fillRoundRect(bx, by, bw, bh, 10, 0x2945);
    tft->drawRoundRect(bx, by, bw, bh, 10, 0xFFFF);
    tft->setFont(&FreeSans12pt7b);
    tft->setTextSize(1);
    tft->setTextColor(0xFFFF);
    const char *line1 = "Applying settings...";
    const char *line2 = "Please wait";
    int16_t tbx, tby;
    uint16_t tbw, tbh;
    tft->getTextBounds(line1, 0, 0, &tbx, &tby, &tbw, &tbh);
    tft->setCursor(bx + (bw - (int) tbw) / 2, by + 36);
    tft->print(line1);
    tft->getTextBounds(line2, 0, 0, &tbx, &tby, &tbw, &tbh);
    tft->setCursor(bx + (bw - (int) tbw) / 2, by + 64);
    tft->print(line2);
}

void SettingsTftForms::discardAndCloseOverlays() {
    closeOverlays(false);
}

bool SettingsTftForms::consumeCancelRequest() {
    if (!cancel_requested)
        return false;
    cancel_requested = false;
    return true;
}

void SettingsTftForms::closeOverlays(bool redraw_footer) {
    if (keyboard && keyboard->isVisible())
        keyboard->close();
    for (int i = 0; i < SETTINGS_SENSOR_FIELDS; i++)
        sensor_fields[i]->getField().setOpen(false);
    editing_kind = 0;
    editing_index = -1;
    if (redraw_footer) {
        drawTabBody();
        drawFooter();
    }
}

bool SettingsTftForms::isOverlayOpen() const {
    if (keyboard && keyboard->isVisible()) return true;
    for (int i = 0; i < SETTINGS_SENSOR_FIELDS; i++)
        if (sensor_fields[i]->getField().isOpen()) return true;
    return false;
}

void SettingsTftForms::placeAndOpenKeyboard(const char *initial, int max_len, KeyboardMode mode,
                                           int field_y, int field_h) {
    const int kb_h = 128;
    const int gap = 4;
    const bool place_top = (field_y + field_h / 2) > (UI_SETTINGS_SCREEN_HEIGHT / 2);
    const int kb_y = place_top ? 0 : (UI_SETTINGS_SCREEN_HEIGHT - kb_h);
    const int visible_top = place_top ? (kb_h + gap) : content_y;
    const int visible_bottom = place_top
                               ? (UI_SETTINGS_SCREEN_HEIGHT - SETTINGS_FOOTER_HEIGHT - gap)
                               : kb_y;

    bool scrolled = false;
    if (field_y < visible_top) {
        scroll_y -= (visible_top - field_y);
        scrolled = true;
    } else if (field_y + field_h > visible_bottom) {
        scroll_y += (field_y + field_h - visible_bottom);
        scrolled = true;
    }
    if (scroll_y < 0) {
        scroll_y = 0;
        scrolled = true;
    }
    int max_scroll = tabContentHeight() - content_h;
    if (max_scroll < 0) max_scroll = 0;
    if (scroll_y > max_scroll) {
        scroll_y = max_scroll;
        scrolled = true;
    }

    if (scrolled) {
        layoutFields();
        redrawScrolledBody();
    }

    keyboard->open(0, kb_y, UI_SETTINGS_SCREEN_WIDTH, kb_h, initial, max_len, mode);
    keyboard->invalidate();
    keyboard->draw();
}

void SettingsTftForms::openKeyboardForText(LabeledTextField *field, int max_len) {
    for (int i = 0; i < SETTINGS_SENSOR_FIELDS; i++)
        sensor_fields[i]->getField().setOpen(false);
    strncpy(edit_backup, field->getValue(), FORM_MAX_VALUE_LEN - 1);
    edit_backup[FORM_MAX_VALUE_LEN - 1] = 0;
    field->setFocused(true);
    field->invalidate();
    field->draw();
    uint16_t fx, fy;
    field->getXY(fx, fy);
    placeAndOpenKeyboard(field->getValue(), max_len, KeyboardMode::ALPHA, (int) fy, field->getTotalHeight());
}

void SettingsTftForms::openKeyboardForNumber(LabeledNumberField *field) {
    for (int i = 0; i < SETTINGS_SENSOR_FIELDS; i++)
        sensor_fields[i]->getField().setOpen(false);
    strncpy(edit_backup, field->getField().getValue(), FORM_MAX_VALUE_LEN - 1);
    edit_backup[FORM_MAX_VALUE_LEN - 1] = 0;
    field->setFocused(true);
    field->invalidate();
    field->draw();
    uint16_t fx, fy;
    field->getXY(fx, fy);
    placeAndOpenKeyboard(field->getField().getValue(), 16, KeyboardMode::NUMERIC, (int) fy,
                         field->getTotalHeight());
}

void SettingsTftForms::openSensorDropdown(int index) {
    if (index < 0 || index >= SETTINGS_SENSOR_FIELDS)
        return;

    for (int j = 0; j < SETTINGS_SENSOR_FIELDS; j++)
        sensor_fields[j]->getField().setOpen(false);

    const int want_list_h = FORM_DROPDOWN_ROW_HEIGHT * SETTINGS_DROPDOWN_VISIBLE_ROWS;
    const int gap = 4;
    const int ft = footerTop();

    uint16_t fx, fy;
    sensor_fields[index]->getXY(fx, fy);
    int field_bottom = (int) fy + sensor_fields[index]->getTotalHeight();
    int need_bottom = field_bottom + want_list_h + gap;
    if (need_bottom > ft) {
        setScrollY(scroll_y + (need_bottom - ft));
        sensor_fields[index]->getXY(fx, fy);
        field_bottom = (int) fy + sensor_fields[index]->getTotalHeight();
    }

    int avail = ft - field_bottom - gap;
    if (avail < FORM_DROPDOWN_ROW_HEIGHT)
        avail = FORM_DROPDOWN_ROW_HEIGHT;
    if (avail > want_list_h)
        avail = want_list_h;

    DropDownList &dd = sensor_fields[index]->getField();
    dd.setListMaxHeight(avail);
    dd.setOpen(true);
    sensor_fields[index]->invalidate();
    sensor_fields[index]->draw();
    dd.drawListOverlay();
    drawFooter();
}

void SettingsTftForms::drawFooter() {
    const int fy = UI_SETTINGS_SCREEN_HEIGHT - SETTINGS_FOOTER_HEIGHT;
    const int third = UI_SETTINGS_SCREEN_WIDTH / 3;
    auto drawBtn = [&](int bx, const char *cap, uint16_t bg, uint16_t fg) {
        tft->fillRoundRect(bx + 2, fy + 2, third - 4, SETTINGS_FOOTER_HEIGHT - 4, 4, bg);
        tft->setFont(&FreeSans9pt7b);
        tft->setTextSize(1);
        tft->setTextColor(fg);
        int16_t tbx, tby;
        uint16_t tbw, tbh;
        tft->getTextBounds(cap, 0, 0, &tbx, &tby, &tbw, &tbh);
        tft->setCursor(bx + (third - (int) tbw) / 2, fy + 18);
        tft->print(cap);
    };
    tft->fillRect(0, fy, UI_SETTINGS_SCREEN_WIDTH, SETTINGS_FOOTER_HEIGHT, UI_SETTINGS_COLOR_GUTTER);
    drawBtn(0, "Reload", 0x6B4D, 0xFFFF);
    drawBtn(third, "Cancel", 0x9800, 0xFCD3);
    drawBtn(2 * third, "Save", 0x0320, 0xCFF7);
}

void SettingsTftForms::drawTabBody() {
    layoutFields();
    auto visible = [&](int top, int h) {
        return top < content_y + content_h
               && top + h > content_y
               && top + h <= content_y + content_h;
    };
    const int item_h = FORM_LABEL_HEIGHT + FORM_LABEL_FIELD_GAP + FORM_FIELD_HEIGHT;

    if (active_tab == UI_SETTINGS_TAB_WIFI) {
        if (visible(fieldTop(0), item_h)) { wifi_ssid->invalidate(); wifi_ssid->draw(); }
        if (visible(fieldTop(1), item_h)) { wifi_password->invalidate(); wifi_password->draw(); }
    } else if (active_tab == UI_SETTINGS_TAB_MQTT) {
        if (visible(fieldTop(0), item_h)) { mqtt_text[0]->invalidate(); mqtt_text[0]->draw(); }
        if (visible(fieldTop(1), item_h)) { mqtt_port->invalidate(); mqtt_port->draw(); }
        if (visible(fieldTop(2), item_h)) { mqtt_reconnect->invalidate(); mqtt_reconnect->draw(); }
        for (int i = 1; i < SETTINGS_MQTT_TEXT; i++) {
            if (visible(fieldTop(i + 2), item_h)) {
                mqtt_text[i]->invalidate();
                mqtt_text[i]->draw();
            }
        }
    } else if (active_tab == UI_SETTINGS_TAB_SENSORS) {
        for (int i = 0; i < SETTINGS_SENSOR_FIELDS; i++) {
            if (visible(fieldTop(i), item_h)) {
                sensor_fields[i]->invalidate();
                sensor_fields[i]->draw();
            }
        }
    } else if (active_tab == UI_SETTINGS_TAB_SERVO) {
        for (int i = 0; i < SETTINGS_SERVO_FIELDS; i++) {
            if (visible(fieldTop(i), item_h)) {
                servo_fields[i]->invalidate();
                servo_fields[i]->draw();
            }
        }
    } else if (active_tab == UI_SETTINGS_TAB_CAPACITIES) {
        if (visible(fieldTop(0), FORM_LABEL_HEIGHT)) {
            capacity_headers[0]->invalidate();
            capacity_headers[0]->draw();
        }
        for (int i = 0; i < SETTINGS_CAPACITY_FIELDS; i++) {
            int slot = capacitySlotForField(i);
            if (visible(fieldTop(slot), item_h)) {
                capacity_fields[i]->invalidate();
                capacity_fields[i]->draw();
            }
        }
    } else if (active_tab == UI_SETTINGS_TAB_RULES) {
        for (int i = 0; i < SETTINGS_RULES_HEADERS; i++) {
            int slot = rulesHeaderSlot(i);
            if (visible(fieldTop(slot), FORM_LABEL_HEIGHT)) {
                rules_headers[i]->invalidate();
                rules_headers[i]->draw();
            }
        }
        for (int i = 0; i < SETTINGS_RULES_FIELDS; i++) {
            int slot = rulesSlotForField(i);
            if (visible(fieldTop(slot), item_h)) {
                rules_fields[i]->invalidate();
                rules_fields[i]->draw();
            }
        }
    } else if (active_tab == UI_SETTINGS_TAB_MODES) {
        for (int i = 0; i < SETTINGS_MODES_HEADERS; i++) {
            int slot = modesHeaderSlot(i);
            if (visible(fieldTop(slot), FORM_LABEL_HEIGHT)) {
                modes_headers[i]->invalidate();
                modes_headers[i]->draw();
            }
        }
        for (int i = 0; i < SETTINGS_MODES_FIELDS; i++) {
            int slot = modesSlotForField(i);
            if (visible(fieldTop(slot), item_h)) {
                modes_fields[i]->invalidate();
                modes_fields[i]->draw();
            }
        }
    }
    drawScrollbar();
}

void SettingsTftForms::drawOverlays() {
    if (keyboard && keyboard->isVisible())
        keyboard->draw();
    for (int i = 0; i < SETTINGS_SENSOR_FIELDS; i++)
        if (sensor_fields[i]->getField().isOpen())
            sensor_fields[i]->getField().drawListOverlay();
}

bool SettingsTftForms::handleTouch(uint16_t tx, uint16_t ty) {
    const int fy = UI_SETTINGS_SCREEN_HEIGHT - SETTINGS_FOOTER_HEIGHT;
    const bool overlay = isOverlayOpen();

    if (keyboard && keyboard->isVisible()) {
        if (keyboard->contains(tx, ty)) {
            if (keyboard->handleTouch(tx, ty))
                keyboard->draw();
            return true;
        }
        dismissKeyboard();
        return true;
    }

    for (int i = 0; i < SETTINGS_SENSOR_FIELDS; i++) {
        DropDownList &dd = sensor_fields[i]->getField();
        if (dd.isOpen()) {
            int opt = -1;
            if (dd.hitTestList(tx, ty, opt)) {
                dd.setSelectedIndex(opt);
                dd.setOpen(false);
                restoreUiChrome();
                return true;
            }
            dd.setOpen(false);
            restoreUiChrome();
            return true;
        }
    }

    if (!overlay && ty >= (uint16_t) fy) {
        int third = UI_SETTINGS_SCREEN_WIDTH / 3;
        int col = tx / third;
        if (col <= 0) {
            reload();
            return true;
        }
        if (col == 1) {
            cancel_requested = true;
            return true;
        }
        if (col >= 2) {
            save();
            return true;
        }
    }

    if (handleScrollbarTouch(tx, ty))
        return true;

    if (tx >= (uint16_t) content_x && tx < (uint16_t) (content_x + content_w)
        && ty >= (uint16_t) content_y && ty < (uint16_t) (content_y + content_h)) {
        int ms = maxScroll();
        if (ty < (uint16_t) (content_y + 24) && scroll_y > 0) {
            setScrollY(scroll_y - 40);
            return true;
        }
        if (ty > (uint16_t) (content_y + content_h - 24) && scroll_y < ms) {
            setScrollY(scroll_y + 40);
            return true;
        }
    }

    if (active_tab == UI_SETTINGS_TAB_WIFI) {
        if (wifi_ssid->hitTest(tx, ty)) {
            editing_kind = 1;
            editing_index = 0;
            openKeyboardForText(wifi_ssid, textMaxLenForParam("network>ssid", 63));
            return true;
        }
        if (wifi_password->hitTest(tx, ty)) {
            editing_kind = 1;
            editing_index = 1;
            openKeyboardForText(wifi_password, textMaxLenForParam("network>password", 63));
            return true;
        }
    } else if (active_tab == UI_SETTINGS_TAB_MQTT) {
        static const char *mqtt_text_params[SETTINGS_MQTT_TEXT] = {
                "mqtt>server",
                "mqtt>deviceName",
                "mqtt>serverBornTopic",
                "device>stateOutgoingTopicPrefix",
                "device>iHaveBornTopic",
                "device>incomingCommandTopicPrefix",
                "device>mqttInputToolTopic",
                "device>mqttOutputToolTopic"
        };
        if (mqtt_text[0]->hitTest(tx, ty)) {
            editing_kind = 1;
            editing_index = 10;
            openKeyboardForText(mqtt_text[0], textMaxLenForParam(mqtt_text_params[0], 63));
            return true;
        }
        if (mqtt_port->hitTest(tx, ty)) {
            editing_kind = 2; editing_index = 0; openKeyboardForNumber(mqtt_port); return true;
        }
        if (mqtt_reconnect->hitTest(tx, ty)) {
            editing_kind = 2; editing_index = 1; openKeyboardForNumber(mqtt_reconnect); return true;
        }
        for (int i = 1; i < SETTINGS_MQTT_TEXT; i++) {
            if (mqtt_text[i]->hitTest(tx, ty)) {
                editing_kind = 1;
                editing_index = 10 + i;
                openKeyboardForText(mqtt_text[i], textMaxLenForParam(mqtt_text_params[i], 63));
                return true;
            }
        }
    } else if (active_tab == UI_SETTINGS_TAB_SENSORS) {
        for (int i = 0; i < SETTINGS_SENSOR_FIELDS; i++) {
            if (sensor_fields[i]->hitTest(tx, ty)) {
                openSensorDropdown(i);
                return true;
            }
        }
    } else if (active_tab == UI_SETTINGS_TAB_SERVO) {
        for (int i = 0; i < SETTINGS_SERVO_FIELDS; i++) {
            if (servo_fields[i]->hitTest(tx, ty)) {
                editing_kind = 2; editing_index = 100 + i; openKeyboardForNumber(servo_fields[i]); return true;
            }
        }
    } else if (active_tab == UI_SETTINGS_TAB_CAPACITIES) {
        for (int i = 0; i < SETTINGS_CAPACITY_FIELDS; i++) {
            if (capacity_fields[i]->hitTest(tx, ty)) {
                editing_kind = 2;
                editing_index = SETTINGS_EDIT_NUM_CAPACITY + i;
                openKeyboardForNumber(capacity_fields[i]);
                return true;
            }
        }
    } else if (active_tab == UI_SETTINGS_TAB_RULES) {
        for (int i = 0; i < SETTINGS_RULES_FIELDS; i++) {
            if (rules_fields[i]->hitTest(tx, ty)) {
                editing_kind = 2;
                editing_index = SETTINGS_EDIT_NUM_RULES + i;
                openKeyboardForNumber(rules_fields[i]);
                return true;
            }
        }
    } else if (active_tab == UI_SETTINGS_TAB_MODES) {
        for (int i = 0; i < SETTINGS_MODES_FIELDS; i++) {
            if (modes_fields[i]->hitTest(tx, ty)) {
                editing_kind = 2;
                editing_index = SETTINGS_EDIT_NUM_MODES + i;
                openKeyboardForNumber(modes_fields[i]);
                return true;
            }
        }
    }
    return false;
}
