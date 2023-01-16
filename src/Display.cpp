//
// Created by dsporykhin on 24.07.22.
//

#include <lib/bufferedGraphics/DisplayBuffer.h>
#include "Display.h"
#include "Defines.h"
#include "lib/adafruit/Fonts/FreeSans12pt7b.h"
#include "lib/adafruit/Fonts/FreeSerif9pt7b.h"
#include "lib/adafruit/Fonts/FreeMono12pt7b.h"
#include "HeaterController.h"
#include "Gauge.h"

Display::Display() {
    telemetry_initialized = false;
    tft = new ILI9488(DISPLAY_CS_PIN, DISPLAY_DC_PIN, DISPLAY_RST_PIN);
    tft->begin();
    tft->setRotation(1);
    tft->fillScreen(0);

    tft->setFont(&FreeSans12pt7b);

    tft->setCursor(0, 0);
    tft->setTextColor(0xff00, 0x00ff);
    tft->fillScreen(COLOR_BACKGROUND);

    defaultDisplayBuffer = new DisplayBuffer(UI_PAGE_0_SIZE_GAUGE_WIDTH, UI_PAGE_0_SIZE_GAUGE_HEIGHT);
    // core temperature
    gauge_core_tempr = new Gauge(tft, "Core T (C)", 0, 0, UI_PAGE_0_SIZE_GAUGE_WIDTH, UI_PAGE_0_SIZE_GAUGE_HEIGHT,
                                 tft->color24To16(UI_PAGE_0_COLOR_GAUGE_BACKGROUND),
                                 0xffffff, NULL, 2, 40, 103, new ColorPart(UI_COLOR_GAUGE_BLUE, 55,
                                                                           new ColorPart(UI_COLOR_GAUGE_GREEN, 90,
                                                                                         new ColorPart(UI_COLOR_GAUGE_YELLOW,
                                                                                                 96, new ColorPart(
                                                                                                         UI_COLOR_GAUGE_RED,
                                                                                                         0, NULL)))));
    gauge_core_tempr->defaultDisplayBuffer = defaultDisplayBuffer;
//    gauge_core_tempr->setValue(30);

    // core power
    gauge_core_power = new Gauge(tft, "Core P(W)", (UI_PAGE_0_SIZE_GAUGE_WIDTH + 3), 0, UI_PAGE_0_SIZE_GAUGE_WIDTH,
                                 UI_PAGE_0_SIZE_GAUGE_HEIGHT,
                                 tft->color24To16(UI_PAGE_0_COLOR_GAUGE_BACKGROUND),
                                 0xffffff, NULL, 0, 3000, 45000, new ColorPart(UI_COLOR_GAUGE_BLUE, 15000,
                                                                               new ColorPart(UI_COLOR_GAUGE_GREEN,
                                                                                             35000,
                                                                                             new ColorPart(UI_COLOR_GAUGE_YELLOW,
                                                                                                     43000,
                                                                                                     new ColorPart(
                                                                                                             UI_COLOR_GAUGE_RED,
                                                                                                             0,
                                                                                                             NULL)))));
    gauge_core_power->defaultDisplayBuffer = defaultDisplayBuffer;
//    gauge_core_power->setValue(23000);

    // warming tempr
    gauge_warm_flow_tempr = new Gauge(tft, "Warm T(C)", (UI_PAGE_0_SIZE_GAUGE_WIDTH + 3) * 2, 0,
                                      UI_PAGE_0_SIZE_GAUGE_WIDTH, UI_PAGE_0_SIZE_GAUGE_HEIGHT,
                                      tft->color24To16(UI_PAGE_0_COLOR_GAUGE_BACKGROUND),
                                      0xffffff, NULL, 2, 30, 90, new ColorPart(UI_COLOR_GAUGE_BLUE, 50,
                                                                               new ColorPart(UI_COLOR_GAUGE_GREEN, 75,
                                                                                             new ColorPart(UI_COLOR_GAUGE_YELLOW,
                                                                                                     83, new ColorPart(
                                                                                                             UI_COLOR_GAUGE_RED,
                                                                                                             0,
                                                                                                             NULL)))));
    gauge_acc_top_tempr->defaultDisplayBuffer = defaultDisplayBuffer;
//    gauge_acc_top_tempr->setValue(55);

    // core output flow tempr
    gauge_core_output_tempr = new Gauge(tft, "Output T", (UI_PAGE_0_SIZE_GAUGE_WIDTH + 3) * 0,
                                        (UI_PAGE_0_SIZE_GAUGE_HEIGHT + 3) * 1, UI_PAGE_0_SIZE_GAUGE_WIDTH,
                                        UI_PAGE_0_SIZE_GAUGE_HEIGHT,
                                        tft->color24To16(UI_PAGE_0_COLOR_GAUGE_BACKGROUND),
                                        0xffffff, NULL, 2, 40, 105, new ColorPart(UI_COLOR_GAUGE_BLUE, 60,
                                                                                  new ColorPart(UI_COLOR_GAUGE_GREEN,
                                                                                                87,
                                                                                                new ColorPart(UI_COLOR_GAUGE_YELLOW,
                                                                                                        96,
                                                                                                        new ColorPart(
                                                                                                                UI_COLOR_GAUGE_RED,
                                                                                                                0,
                                                                                                                NULL)))));
    gauge_core_output_tempr->defaultDisplayBuffer = defaultDisplayBuffer;
//    gauge_core_output_tempr->setValue(68);


    // power balance
    gauge_power_balance = new Gauge(tft, "Pwr P(kW)", (UI_PAGE_0_SIZE_GAUGE_WIDTH + 3) * 1,
                                    (UI_PAGE_0_SIZE_GAUGE_HEIGHT + 3) * 1, UI_PAGE_0_SIZE_GAUGE_WIDTH,
                                    UI_PAGE_0_SIZE_GAUGE_HEIGHT,
                                    tft->color24To16(UI_PAGE_0_COLOR_GAUGE_BACKGROUND),
                                    0xffffff, NULL, 1, -20, 40, new ColorPart(UI_COLOR_GAUGE_BLUE, 0,
                                                                              new ColorPart(UI_COLOR_GAUGE_RED, 0, NULL)));
    gauge_power_balance->defaultDisplayBuffer = defaultDisplayBuffer;
//    gauge_power_balance->setValue(-4);


    // acc top
    gauge_acc_top_tempr = new Gauge(tft, "Top T(C)", (UI_PAGE_0_SIZE_GAUGE_WIDTH + 3) * 2,
                                    (UI_PAGE_0_SIZE_GAUGE_HEIGHT + 3) * 1, UI_PAGE_0_SIZE_GAUGE_WIDTH,
                                    UI_PAGE_0_SIZE_GAUGE_HEIGHT,
                                    tft->color24To16(UI_PAGE_0_COLOR_GAUGE_BACKGROUND),
                                    0xffffff, NULL, 2, 30, 90, new ColorPart(UI_COLOR_GAUGE_BLUE, 45,
                                                                             new ColorPart(UI_COLOR_GAUGE_GREEN, 75,
                                                                                           new ColorPart(UI_COLOR_GAUGE_YELLOW,
                                                                                                   80, new ColorPart(
                                                                                                           UI_COLOR_GAUGE_RED,
                                                                                                           0, NULL)))));
    gauge_acc_top_tempr->defaultDisplayBuffer = defaultDisplayBuffer;
//    gauge_acc_top_tempr->setValue(68);

    // input flow tempr
    gauge_core_input_tempr = new Gauge(tft, "Input T", (UI_PAGE_0_SIZE_GAUGE_WIDTH + 3) * 0,
                                       (UI_PAGE_0_SIZE_GAUGE_HEIGHT + 3) * 2, UI_PAGE_0_SIZE_GAUGE_WIDTH,
                                       UI_PAGE_0_SIZE_GAUGE_HEIGHT,
                                       tft->color24To16(UI_PAGE_0_COLOR_GAUGE_BACKGROUND),
                                       0xffffff, NULL, 2, 30, 85, new ColorPart(UI_COLOR_GAUGE_BLUE, 50,
                                                                                new ColorPart(UI_COLOR_GAUGE_GREEN, 75,
                                                                                              new ColorPart(UI_COLOR_GAUGE_YELLOW,
                                                                                                      83, new ColorPart(
                                                                                                              UI_COLOR_GAUGE_RED,
                                                                                                              0, NULL)))));
    gauge_core_input_tempr->defaultDisplayBuffer = defaultDisplayBuffer;
//    gauge_core_input_tempr->setValue(54);

    // accumulated power
    gauge_power = new Gauge(tft, "Energy kWh", (UI_PAGE_0_SIZE_GAUGE_WIDTH + 3) * 1,
                                       (UI_PAGE_0_SIZE_GAUGE_HEIGHT + 3) * 2, UI_PAGE_0_SIZE_GAUGE_WIDTH,
                                       UI_PAGE_0_SIZE_GAUGE_HEIGHT,
                                       tft->color24To16(UI_PAGE_0_COLOR_GAUGE_BACKGROUND),
                                       0xffffff, NULL, 2, -15, 70, new ColorPart(UI_COLOR_GAUGE_BLUE, 0,
                                                                                new ColorPart(UI_COLOR_GAUGE_GREEN, 56,
                                                                                              new ColorPart(UI_COLOR_GAUGE_YELLOW,
                                                                                                      65, new ColorPart(
                                                                                                              UI_COLOR_GAUGE_RED,
                                                                                                              0, NULL)))));
    gauge_power->defaultDisplayBuffer = defaultDisplayBuffer;
//    gauge_power->setValue(23.567);

    // acc bottom
    gauge_acc_bottom_tempr = new Gauge(tft, "Bottom T", (UI_PAGE_0_SIZE_GAUGE_WIDTH + 3) * 2,
                                    (UI_PAGE_0_SIZE_GAUGE_HEIGHT + 3) * 2, UI_PAGE_0_SIZE_GAUGE_WIDTH,
                                    UI_PAGE_0_SIZE_GAUGE_HEIGHT,
                                    tft->color24To16(UI_PAGE_0_COLOR_GAUGE_BACKGROUND),
                                    0xffffff, NULL, 2, 30, 80, new ColorPart(UI_COLOR_GAUGE_BLUE, 35,
                                                                             new ColorPart(UI_COLOR_GAUGE_GREEN, 68,
                                                                                           new ColorPart(UI_COLOR_GAUGE_YELLOW,
                                                                                                   75, new ColorPart(
                                                                                                           UI_COLOR_GAUGE_RED,
                                                                                                           0, NULL)))));
    gauge_acc_bottom_tempr->defaultDisplayBuffer = defaultDisplayBuffer;
//    gauge_acc_bottom_tempr->setValue(56);
}

void Display::printStatus(const char *status) {
    tft->setTextColor(SCREEN_COLOR_LIGHT_RED);
    tft->println(status);
}

void Display::updateInfo(TelemetryDataRecord *telemetryDataRecord) {
    LOGGER.info("drawScreen0(telemetryDataRecord)");
    Serial.flush();

    drawScreen0(telemetryDataRecord);
    memcpy(&this->savedDataRecord, telemetryDataRecord, sizeof(TelemetryDataRecord));
    telemetry_initialized = true;
}

void Display::drawScreen() {
    switch (screen_index) {
        case 0:
            drawScreen0(&savedDataRecord);
            break;
    }
}

void Display::setScreenIndex(int index) {
    if (screen_index != index) {
        screen_index = index;
        switch (screen_index) {
            case 0:
                initScreen0();
                break;
        }
    }
}

void Display::drawFloatField(const char *msg, float value, int txt_x, int txt_y, int width, int font_color,
                             int bg_color) {
    char buffer[32];
    sprintf(buffer, msg, value);
    drawField(buffer, txt_x, txt_y, width, font_color, bg_color);
}

void Display::drawIntField(const char *msg, int value, int txt_x, int txt_y, int width, int font_color, int bg_color) {
    char buffer[32];
    sprintf(buffer, msg, value);
    drawField(buffer, txt_x, txt_y, width, font_color, bg_color);
}

void Display::drawField(const char *msg, int txt_x, int txt_y, int width, int font_color, int bg_color) {
    tft->fillRect(txt_x, txt_y - 17, width, 19, bg_color);
    tft->setCursor(txt_x, txt_y);
    tft->setTextColor(font_color);
    tft->print(msg);
}

void Display::drawGauges() {
    gauge_core_tempr->draw();
    gauge_acc_bottom_tempr->draw();
    gauge_warm_flow_tempr->draw();
    gauge_core_input_tempr->draw();
    gauge_power_balance->draw();
    gauge_core_output_tempr->draw();
    gauge_acc_top_tempr->draw();
    gauge_core_power->draw();
    gauge_power->draw();
}

void Display::initScreen0() {
    tft->fillScreen(UI_PAGE_0_COLOR_MAIN_BACKGROUND);

    drawGauges();

    if (telemetry_initialized)
        drawScreen0(&savedDataRecord);
}

void Display::drawScreen0(TelemetryDataRecord *telemetryDataRecord) {

LOGGER.info("drawScreen0 1");
Serial.flush();

    gauge_core_tempr->setValue(telemetryDataRecord->core_temp_sma);
    gauge_core_power->setValue(telemetryDataRecord->core_EMA_power);
    gauge_warm_flow_tempr->setValue(telemetryDataRecord->forwar_flow_temp_sma);

LOGGER.info("drawScreen0 2");
Serial.flush();
    gauge_core_output_tempr->setValue(telemetryDataRecord->output_temp_sma);

LOGGER.info("drawScreen0 3");
Serial.flush();
    if (telemetryDataRecord->power_balance_ready)
        gauge_power_balance->setValue(telemetryDataRecord->power_balance_kwt_hour);

    gauge_acc_top_tempr->setValue(telemetryDataRecord->accumulator_top_temp_sma);
LOGGER.info("drawScreen0 4");
Serial.flush();

    gauge_core_input_tempr->setValue(telemetryDataRecord->input_temp_sma);
    gauge_power->setValue(telemetryDataRecord->accumulated_energy_kwt_hour);
    gauge_acc_bottom_tempr->setValue(telemetryDataRecord->accumulator_bottom_temp_sma);

LOGGER.info("before drawGauges");
Serial.flush();
    drawGauges();

//    char buffer[30];
//
//    tft->setFont(&FreeSans12pt7b);
//
//    drawIntField("%d%%", (int) telemetryDataRecord->smoke_door_position, 5, 20, 97, SCREEN_COLOR_GRAY, COLOR_CORE);
//    drawFloatField("%0.2fC", telemetryDataRecord->output_temp_sma, 5, 45, 77, ST77XX_RED, COLOR_CORE);
//    drawIntField("%d L/m", (int) telemetryDataRecord->core_flow, 5, 64, 77, ST77XX_RED, COLOR_CORE);
//
//    drawIntField("%d%%", (int) telemetryDataRecord->upper_door_position, 5, 89, 77, SCREEN_COLOR_CYAN, COLOR_CORE);
//    drawFloatField("%0.2fC", telemetryDataRecord->core_temp_sma, 5, 108, 77, SCREEN_COLOR_ORANGE, COLOR_CORE);
//    drawIntField("%dW", (int) telemetryDataRecord->core_EMA_power, 5, 127, 97, SCREEN_COLOR_ORANGE, COLOR_CORE);
//
//    drawFloatField("%0.2fC", telemetryDataRecord->input_temp_sma, 5, 152, 77, ST77XX_BLUE, COLOR_CORE);
//    drawIntField("%d%%", (int) telemetryDataRecord->oxygen_door_position, 5, 171, 77, SCREEN_COLOR_CYAN, COLOR_CORE);
//
//    if (telemetryDataRecord->time_to_close_oxygen_door_in_stanby_mode
//        && (telemetryDataRecord->time_to_close_oxygen_door_in_stanby_mode > millis())) {
//
//        int min = (telemetryDataRecord->time_to_close_oxygen_door_in_stanby_mode - millis()) / 1000;
//        int sec = min % 60;
//        min = min / 60;
//        sprintf(buffer, "%2d:%2d", min, sec);
//        drawField(buffer, 5, 193, 97, SCREEN_GREEN, COLOR_CORE);
//    } else
//        drawField("", 5, 193, 97, SCREEN_GREEN, COLOR_CORE);
//
//
//    if (telemetryDataRecord->main_door_opened)
//        drawField("OPENED", 5, 212, 100, SCREEN_COLOR_LIGHT_RED, COLOR_CORE);
//    else
//        drawField("Closed", 5, 212, 100, SCREEN_GREEN, COLOR_CORE);
//
//    tft->fillRect(5, 214, 97, 24, COLOR_CORE);
//    tft->setCursor(5, 231);
//    tft->setTextColor(SCREEN_GREEN);
//    switch (telemetryDataRecord->heaterMode) {
//        case STAND_BY:
//            tft->print("Stand By");
//            break;
//        case WARMING:
//            tft->print("Warming");
//            break;
//        case FINAL_COOLING:
//            tft->print("Cooling");
//            break;
//        case PID:
//            tft->print("Burning");
//            break;
//        case OVERHEATED:
//            tft->setTextColor(ST77XX_YELLOW);
//            tft->print("Overheat");
//            break;
//        case CRITICAL:
//            tft->setTextColor(SCREEN_COLOR_LIGHT_RED);
//            tft->print("Critical");
//            break;
//    }
//
//    drawFloatField("%0.2fC", telemetryDataRecord->accumulator_top_temp_sma, 130, 32, 80, SCREEN_GREEN,
//                   COLOR_ACCUMULATOR);
//    drawFloatField("%0.2fC", telemetryDataRecord->accumulator_higher_temp_sma, 130, 59, 80, SCREEN_GREEN,
//                   COLOR_ACCUMULATOR);
//    drawFloatField("%0.2fC", telemetryDataRecord->accumulator_lower_temp_sma, 130, 85, 80, SCREEN_GREEN,
//                   COLOR_ACCUMULATOR);
//    drawFloatField("%0.2fC", telemetryDataRecord->accumulator_bottom_temp_sma, 130, 112, 80, SCREEN_GREEN,
//                   COLOR_ACCUMULATOR);
//
//    drawPumpState(0, telemetryDataRecord->pump_1_state);
//    drawPumpState(1, telemetryDataRecord->pump_2_state);
//
//    drawFloatField("%0.2fC", telemetryDataRecord->internal_temp, 225, 175, 87, SCREEN_COLOR_GRAY, COLOR_CONTROLLER);
//    drawIntField("%d", (int) telemetryDataRecord->pid_p, 225, 194, 87, SCREEN_COLOR_GRAY, COLOR_CONTROLLER);
//    drawFloatField("%0.1f", telemetryDataRecord->pid_i, 225, 213, 87, SCREEN_COLOR_GRAY, COLOR_CONTROLLER);
//    drawFloatField("%0.1f", telemetryDataRecord->pid_d, 225, 232, 87, SCREEN_COLOR_GRAY, COLOR_CONTROLLER);
//
//    drawFloatField("%0.2fC", telemetryDataRecord->forwar_flow_temp_sma, 236, 33, 77, ST77XX_RED,
//                   SCREEN_COLOR_LIGHT_LIGHT_RED);
//
//    drawFloatField("%0.2fC", telemetryDataRecord->forwar_flow_temp_sma, 236, 86, 77, ST77XX_RED,
//                   SCREEN_COLOR_LIGHT_LIGHT_RED);
//    drawFloatField("%0.1f L/m", telemetryDataRecord->avarage_backward_flow, 236, 110, 77, ST77XX_BLUE,
//                   SCREEN_COLOR_LIGHT_LIGHT_RED);
//    drawFloatField("%0.2fC", telemetryDataRecord->backward_flow_temp_sma, 236, 134, 77, ST77XX_BLUE,
//                   SCREEN_COLOR_LIGHT_LIGHT_RED);
//
//    if (telemetryDataRecord->power_balance_ready) {
//        int _color = telemetryDataRecord->power_balance_kwt_hour > 0
//                     ? ST77XX_RED
//                     : ST77XX_BLUE;
//        drawFloatField("%0.2fkw", telemetryDataRecord->power_balance_kwt_hour, 112, 196, 100, _color,
//                       COLOR_ACCUMULATED);
//    } else
//        drawFloatField("%0.2fkw", 0, 112, 196, 100, ST77XX_BLUE, COLOR_ACCUMULATED);
//
//    drawFloatField("%0.2fkwh", telemetryDataRecord->accumulated_energy_kwt_hour, 112, 216, 100, ST77XX_RED,
//                   COLOR_ACCUMULATED);
//
//}
//
//void Display::drawPumpState(int index, bool is_on) {
//    int y = 145;
//    int x = 145;
//    int outer_color, inner_color;
//    if (is_on) {
//        outer_color = SCREEN_GREEN;
//        inner_color = SCREEN_LIGHT_LIGHT_GREEN;
//    } else {
//        outer_color = ST77XX_RED;
//        inner_color = SCREEN_COLOR_LIGHT_LIGHT_RED;
//    }
//    tft->fillCircle(x + index * 45, y, 16, outer_color);
//    tft->fillCircle(x + index * 45, y, 14, inner_color);
//    tft->fillTriangle(x + 4 + index * 45, y - 7, x - 10 + index * 45, y, x + 4 + index * 45, y + 7, outer_color);
}

ILI9488 *Display::getTft() {
    return tft;
}