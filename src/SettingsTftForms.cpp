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
#include "Defines.h"
#include "Converter.h"
#include "HeaterSettings.h"
#include <lib/ui/form/FormColors.h>

#define UI_SETTINGS_SCREEN_WIDTH 480
#define UI_SETTINGS_SCREEN_HEIGHT 320
#define UI_SETTINGS_COLOR_PAGE 0xEF7D
#define UI_SETTINGS_COLOR_GUTTER 0xE71C
#define UI_SETTINGS_CONTENT_RADIUS 8

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

SettingsTftForms::SettingsTftForms(ILI9488 *tft)
        : tft(tft), settingsManager(nullptr), sensorController(nullptr), wiFiController(nullptr),
          keyboard(nullptr), scroll_y(0), content_x(0), content_y(0), content_w(100), content_h(100),
          active_tab(1), editing_kind(0), editing_index(-1), cancel_requested(false) {
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

    // Smoke → Oxygen → Upper; limits applied later from SettingsNavigator ParamDescriptors.
    const char *servo_doors[3] = {"Smoke", "Oxygen", "Upper"};
    const char *servo_base_lbl[5] = {
            "Min impulse us",
            "Max impulse us",
            "Total degrees",
            "Work min angle",
            "Work max angle"
    };
    bool is_float[5] = {false, false, true, false, false};
    int idx = 0;
    for (int d = 0; d < 3; d++) {
        for (int f = 0; f < 5; f++) {
            char lbl[64];
            snprintf(lbl, sizeof(lbl), "%s: %s", servo_doors[d], servo_base_lbl[f]);
            servo_fields[idx] = new LabeledNumberField(tft, 0, 0, 100, lbl, 0, 1, is_float[f]);
            idx++;
        }
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
    delete keyboard;
}

void SettingsTftForms::setSettingsManager(SettingsManager *sm) {
    settingsManager = sm;
    applyLimitsFromNavigator();
}
void SettingsTftForms::setSensorController(SensorController *sc) { sensorController = sc; }
void SettingsTftForms::setWiFiController(WiFiController *wc) { wiFiController = wc; }
void SettingsTftForms::setChromeRedraw(std::function<void()> fn) { chrome_redraw = fn; }

void SettingsTftForms::applyNumberLimits(LabeledNumberField *field, const char *param_name, const char *base_label) {
    if (!field || !settingsManager || !settingsManager->getNavigator())
        return;
    ParamDescriptor *d = settingsManager->getNavigator()->findParamDescriptor(param_name);
    if (!d)
        return;
    const bool as_float = (d->paramType == FLOAT);
    field->setRange(d->minValue, d->maxValue, as_float);
    char lbl[80];
    if (as_float)
        snprintf(lbl, sizeof(lbl), "%s (%.2f..%.2f)", base_label, d->minValue, d->maxValue);
    else
        snprintf(lbl, sizeof(lbl), "%s (%d..%d)", base_label, (int) d->minValue, (int) d->maxValue);
    field->setLabel(lbl);
}

int SettingsTftForms::textMaxLenForParam(const char *param_name, int fallback) const {
    if (!settingsManager || !settingsManager->getNavigator() || !param_name)
        return fallback;
    ParamDescriptor *d = settingsManager->getNavigator()->findParamDescriptor(param_name);
    if (!d)
        return fallback;
    int max_len = (int) d->maxValue;
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

    // TFT servo order: Smoke → Oxygen → Upper (matches SettingsTftForms field layout).
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

    int idx = 0;
    for (int d = 0; d < 3; d++) {
        for (int f = 0; f < 5; f++) {
            char param[80];
            char base[80];
            snprintf(param, sizeof(param), "heater>servos>%s>%s", door_keys[d], field_keys[f]);
            snprintf(base, sizeof(base), "%s: %s", door_titles[d], field_titles[f]);
            applyNumberLimits(servo_fields[idx], param, base);
            idx++;
        }
    }
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
    } else if (editing_kind == 2 && editing_index == 0) {
        mqtt_port->getField().setValue(text);
    } else if (editing_kind == 2 && editing_index == 1) {
        mqtt_reconnect->getField().setValue(text);
    } else if (editing_kind == 2 && editing_index >= 100 && editing_index < 100 + SETTINGS_SERVO_FIELDS) {
        servo_fields[editing_index - 100]->getField().setValue(text);
    }
}

void SettingsTftForms::redrawEditingField() {
    // Redraw only the value box (not the label stack) so we never paint over the keyboard.
    if (editing_kind == 1 && editing_index == 0) {
        wifi_ssid->getField().invalidate();
        wifi_ssid->getField().draw();
    } else if (editing_kind == 1 && editing_index == 1) {
        wifi_password->getField().invalidate();
        wifi_password->getField().draw();
    } else if (editing_kind == 1 && editing_index >= 10 && editing_index < 10 + SETTINGS_MQTT_TEXT) {
        mqtt_text[editing_index - 10]->getField().invalidate();
        mqtt_text[editing_index - 10]->getField().draw();
    } else if (editing_kind == 2 && editing_index == 0) {
        mqtt_port->getField().invalidate();
        mqtt_port->getField().draw();
    } else if (editing_kind == 2 && editing_index == 1) {
        mqtt_reconnect->getField().invalidate();
        mqtt_reconnect->getField().draw();
    } else if (editing_kind == 2 && editing_index >= 100 && editing_index < 100 + SETTINGS_SERVO_FIELDS) {
        servo_fields[editing_index - 100]->getField().invalidate();
        servo_fields[editing_index - 100]->getField().draw();
    }
}

void SettingsTftForms::dismissKeyboard() {
    if (!keyboard || !keyboard->isVisible())
        return;

    // Clamp number fields on exit.
    if (editing_kind == 2 && editing_index == 0)
        mqtt_port->getField().commitText(keyboard->getText());
    else if (editing_kind == 2 && editing_index == 1)
        mqtt_reconnect->getField().commitText(keyboard->getText());
    else if (editing_kind == 2 && editing_index >= 100 && editing_index < 100 + SETTINGS_SERVO_FIELDS)
        servo_fields[editing_index - 100]->getField().commitText(keyboard->getText());

    if (editing_kind == 1 && editing_index == 0) wifi_ssid->setFocused(false);
    else if (editing_kind == 1 && editing_index == 1) wifi_password->setFocused(false);
    else if (editing_kind == 1 && editing_index >= 10 && editing_index < 10 + SETTINGS_MQTT_TEXT)
        mqtt_text[editing_index - 10]->setFocused(false);
    else if (editing_kind == 2 && editing_index == 0) mqtt_port->setFocused(false);
    else if (editing_kind == 2 && editing_index == 1) mqtt_reconnect->setFocused(false);
    else if (editing_kind == 2 && editing_index >= 100) servo_fields[editing_index - 100]->setFocused(false);

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
    const int gap = 8;
    const int item_h = FORM_LABEL_HEIGHT + FORM_LABEL_FIELD_GAP + FORM_FIELD_HEIGHT + gap;
    int h = 8 + count * item_h;
    // Extra room under last field so a dropdown list can open without covering the footer.
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
    // Slightly wider hit area for fingers
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
    int fx = content_x + pad;
    int fw = content_w - pad * 2;
    if (needsScrollbar())
        fw -= (SETTINGS_SCROLLBAR_WIDTH + SETTINGS_SCROLLBAR_MARGIN);
    if (fw < 40) fw = 40;

    auto place = [&](auto *widget, int idx) {
        int fy = fieldTop(idx);
        widget->setWidth(fw);
        widget->setPosition(fx, fy);
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
    }
}

void SettingsTftForms::loadDraftsFromSettings() {
    if (!settingsManager) return;
    GlobalSettings *s = settingsManager->getSettings();
    wifi_ssid->setValue(s->network.ssid);
    wifi_password->setValue(s->network.password);

    mqtt_text[0]->setValue(s->mqttServer);
    mqtt_port->setNumber((float) s->mqttPort);
    mqtt_reconnect->setNumber((float) s->mqttReconnectIntervalMs);
    mqtt_text[1]->setValue(s->mqttDeviceName);
    mqtt_text[2]->setValue(s->mqttServerBornTopic);
    mqtt_text[3]->setValue(s->deviceStateOutgoingTopicPrefix);
    mqtt_text[4]->setValue(s->deviceIHaveBornTopic);
    mqtt_text[5]->setValue(s->deviceIncomingCommandTopicPrefix);
    mqtt_text[6]->setValue(s->mqttInputToolTopic);
    mqtt_text[7]->setValue(s->mqttOutputToolTopic);

    // sensors options
    for (int i = 0; i < SETTINGS_SENSOR_FIELDS; i++) {
        DropDownList &dd = sensor_fields[i]->getField();
        dd.clearOptions();
        dd.addOption("0000000000000000");
        if (sensorController) {
            char buf[SENSORS_ADDR_SIZE * 2 + 1];
            for (int j = 0; j < sensorController->found_sensors_count && j < FORM_DROPDOWN_MAX_OPTIONS - 1; j++) {
                Converter::bytesToAsciiHex(buf, &sensorController->found_sensors_addr[j * SENSORS_ADDR_SIZE],
                                           SENSORS_ADDR_SIZE);
                dd.addOption(buf);
            }
        }
        char cur[SENSORS_ADDR_SIZE * 2 + 1];
        Converter::bytesToAsciiHex(cur, (uint8_t *) &s->ds18D20Addresses[SENSOR_SETTING_INDEX[i] * SENSORS_ADDR_SIZE],
                                   SENSORS_ADDR_SIZE);
        dd.setSelectedValue(cur);
        dd.setOpen(false);
    }

    ServosHardwareSettings *srv = &s->heaterSettings.servos_hardware_settings;
    ServoHardwareSettings *doors[3] = {
            &srv->smoke_servo_settings, &srv->oxygen_servo_settings, &srv->upper_door_servo_settings
    };
    int idx = 0;
    for (int d = 0; d < 3; d++) {
        servo_fields[idx++]->setNumber((float) doors[d]->min_impulse_length_us);
        servo_fields[idx++]->setNumber((float) doors[d]->max_impulse_length_us);
        servo_fields[idx++]->setNumber(doors[d]->total_degrees);
        servo_fields[idx++]->setNumber((float) doors[d]->working_min_angle);
        servo_fields[idx++]->setNumber((float) doors[d]->working_max_angle);
    }
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

    GlobalSettings *s = settingsManager->getSettings();

    strncpy(s->network.ssid, wifi_ssid->getValue(), sizeof(s->network.ssid) - 1);
    s->network.ssid[sizeof(s->network.ssid) - 1] = 0;
    strncpy(s->network.password, wifi_password->getValue(), sizeof(s->network.password) - 1);
    s->network.password[sizeof(s->network.password) - 1] = 0;

    strncpy(s->mqttServer, mqtt_text[0]->getValue(), sizeof(s->mqttServer) - 1);
    s->mqttServer[sizeof(s->mqttServer) - 1] = 0;
    s->mqttPort = (int) mqtt_port->getNumber();
    s->mqttReconnectIntervalMs = (long) mqtt_reconnect->getNumber();
    strncpy(s->mqttDeviceName, mqtt_text[1]->getValue(), sizeof(s->mqttDeviceName) - 1);
    s->mqttDeviceName[sizeof(s->mqttDeviceName) - 1] = 0;
    strncpy(s->mqttServerBornTopic, mqtt_text[2]->getValue(), sizeof(s->mqttServerBornTopic) - 1);
    s->mqttServerBornTopic[sizeof(s->mqttServerBornTopic) - 1] = 0;
    strncpy(s->deviceStateOutgoingTopicPrefix, mqtt_text[3]->getValue(), sizeof(s->deviceStateOutgoingTopicPrefix) - 1);
    s->deviceStateOutgoingTopicPrefix[sizeof(s->deviceStateOutgoingTopicPrefix) - 1] = 0;
    strncpy(s->deviceIHaveBornTopic, mqtt_text[4]->getValue(), sizeof(s->deviceIHaveBornTopic) - 1);
    s->deviceIHaveBornTopic[sizeof(s->deviceIHaveBornTopic) - 1] = 0;
    strncpy(s->deviceIncomingCommandTopicPrefix, mqtt_text[5]->getValue(),
            sizeof(s->deviceIncomingCommandTopicPrefix) - 1);
    s->deviceIncomingCommandTopicPrefix[sizeof(s->deviceIncomingCommandTopicPrefix) - 1] = 0;
    strncpy(s->mqttInputToolTopic, mqtt_text[6]->getValue(), sizeof(s->mqttInputToolTopic) - 1);
    s->mqttInputToolTopic[sizeof(s->mqttInputToolTopic) - 1] = 0;
    strncpy(s->mqttOutputToolTopic, mqtt_text[7]->getValue(), sizeof(s->mqttOutputToolTopic) - 1);
    s->mqttOutputToolTopic[sizeof(s->mqttOutputToolTopic) - 1] = 0;

    for (int i = 0; i < SETTINGS_SENSOR_FIELDS; i++) {
        const char *hex = sensor_fields[i]->getField().getSelectedValue();
        Converter::asciiHexToBytes(
                (uint8_t *) &s->ds18D20Addresses[SENSOR_SETTING_INDEX[i] * SENSORS_ADDR_SIZE],
                hex, SENSORS_ADDR_SIZE);
    }

    ServosHardwareSettings *srv = &s->heaterSettings.servos_hardware_settings;
    ServoHardwareSettings *doors[3] = {
            &srv->smoke_servo_settings, &srv->oxygen_servo_settings, &srv->upper_door_servo_settings
    };
    int idx = 0;
    for (int d = 0; d < 3; d++) {
        doors[d]->min_impulse_length_us = (int) servo_fields[idx++]->getNumber();
        doors[d]->max_impulse_length_us = (int) servo_fields[idx++]->getNumber();
        doors[d]->total_degrees = servo_fields[idx++]->getNumber();
        doors[d]->working_min_angle = (int) servo_fields[idx++]->getNumber();
        doors[d]->working_max_angle = (int) servo_fields[idx++]->getNumber();
    }

    settingsManager->saveSetting(false);
    if (wiFiController)
        wiFiController->reapplyNetworkSettings();

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
    // Lower-half fields → keyboard on top; upper-half → keyboard on bottom.
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
    // Never draw fields that would spill past the content panel (into the footer).
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
    // footer first if no overlay covering
    const int fy = UI_SETTINGS_SCREEN_HEIGHT - SETTINGS_FOOTER_HEIGHT;
    const bool overlay = isOverlayOpen();

    if (keyboard && keyboard->isVisible()) {
        if (keyboard->contains(tx, ty)) {
            if (keyboard->handleTouch(tx, ty))
                keyboard->draw(); // no-op unless shift/symbols layout changed
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
            // tap outside closes
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

    // Vertical scrollbar (when content overflows)
    if (handleScrollbarTouch(tx, ty))
        return true;

    // Tap near top/bottom of content to page-scroll
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
    }
    return false;
}
