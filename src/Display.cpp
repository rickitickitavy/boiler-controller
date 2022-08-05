//
// Created by dsporykhin on 24.07.22.
//

#include "Display.h"
#include "Defines.h"
#include "lib/adafruit/Fonts/FreeSans12pt7b.h"
#include "lib/adafruit/Fonts/FreeSerif9pt7b.h"
#include "lib/adafruit/Fonts/FreeMono12pt7b.h"
#include "HeaterController.h"

Display::Display() {
    telemetry_initialized = false;
    tft = new Adafruit_ST7789(DISPLAY_CS_PIN, DISPLAY_DC_PIN, DISPLAY_RST_PIN);
    tft->init(240, 320, SPI_MODE0);
    tft->setRotation(3);
    tft->fillScreen(0);

    tft->setFont(&FreeSans12pt7b);

    tft->setCursor(0, 0);
    tft->setTextColor(0xff00, 0x00ff);
}

void Display::printStatus(const char *status) {
    tft->setTextColor(SCREEN_COLOR_LIGHT_RED);
    tft->println(status);
}

void Display::updateInfo(TelemetryDataRecord *telemetryDataRecord) {
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

void Display::initScreen0() {
    tft->fillScreen(COLOR_BACKGROUND);
//    tft->fillScreen(SCREEN_COLOR_GRAY);
    tft->drawRect(1, 1, 105, 239, ST77XX_BLACK);
    tft->fillRect(2, 2, 103, 237, COLOR_CORE);
    tft->setFont(&FreeSans12pt7b);

    tft->setCursor(5, 20);

    tft->fillTriangle(82, 38, 102, 48, 82, 58, ST77XX_RED);

    tft->drawRect(105, 136, 20, 20, ST77XX_BLUE);
    tft->fillRect(106, 137, 18, 18, SCREEN_COLOR_LIGHT_LIGHT_BLUE);

    tft->drawRect(105, 40, 20, 20, ST77XX_RED);
    tft->fillRect(106, 41, 18, 18, SCREEN_LIGHT_LIGHT_RED);

    tft->drawRoundRect(125, 10, 88, 160, 10, COLOR_CONTOUR_ACCUMULATOR);
    tft->fillRoundRect(126, 11, 86, 158, 10, COLOR_ACCUMULATOR);

    tft->drawCircle(135, 200, 20, SCREEN_GREEN);
    tft->drawCircle(135, 200, 19, SCREEN_GREEN);
    tft->fillCircle(135, 200, 18, SCREEN_LIGHT_LIGHT_GREEN);
    tft->fillTriangle(142, 190, 122, 200, 142, 210, SCREEN_GREEN);
    tft->fillTriangle(100, 136, 81, 146, 100, 156, ST77XX_BLUE);

    tft->drawCircle(185, 200, 20, SCREEN_GREEN);
    tft->drawCircle(185, 200, 19, SCREEN_GREEN);
    tft->fillCircle(185, 200, 18, SCREEN_LIGHT_LIGHT_GREEN);
    tft->fillTriangle(192, 190, 172, 200, 192, 210, SCREEN_GREEN);

    tft->drawRoundRect(220, 154, 96, 85, 10, COLOR_CONTOUR_CONTROLLER);
    tft->fillRoundRect(221, 155, 94, 83, 10, COLOR_CONTROLLER);

    if (telemetry_initialized)
        drawScreen0(&savedDataRecord);
}

void Display::drawScreen0(TelemetryDataRecord *telemetryDataRecord) {
    char buffer[30];

    tft->setFont(&FreeSans12pt7b);

    drawIntField("%d%%", (int) telemetryDataRecord->smoke_door_position, 5, 20, 97, SCREEN_COLOR_GRAY, COLOR_CORE);
    drawFloatField("%0.2fC", telemetryDataRecord->output_temp_sma, 5, 45, 77, ST77XX_RED, COLOR_CORE);
    drawIntField("%d L/m", (int) telemetryDataRecord->core_flow, 5, 64, 77, ST77XX_RED, COLOR_CORE);

    drawIntField("%d%%", (int) telemetryDataRecord->upper_door_position, 5, 89, 77, SCREEN_COLOR_CYAN, COLOR_CORE);
    drawFloatField("%0.2fC", telemetryDataRecord->core_temp_sma, 5, 108, 77, SCREEN_COLOR_ORANGE, COLOR_CORE);
    drawIntField("%dW", (int) telemetryDataRecord->core_EMA_power, 5, 127, 97, SCREEN_COLOR_ORANGE, COLOR_CORE);

    drawFloatField("%0.2fC", telemetryDataRecord->input_temp_sma, 5, 152, 77, ST77XX_BLUE, COLOR_CORE);
    drawIntField("%d%%", (int) telemetryDataRecord->oxygen_door_position, 5, 171, 77, SCREEN_COLOR_CYAN, COLOR_CORE);

    if (telemetryDataRecord->time_to_close_oxygen_door_in_stanby_mode
        && (telemetryDataRecord->time_to_close_oxygen_door_in_stanby_mode > millis())){

        int min = (telemetryDataRecord->time_to_close_oxygen_door_in_stanby_mode - millis()) / 1000;
        int sec = min % 60;
        min = min / 60;
        sprintf(buffer, "%2d:%2d", min, sec);
        drawField(buffer, 5, 193, 97, SCREEN_GREEN, COLOR_CORE);
    }
    else
        drawField("", 5, 193, 97, SCREEN_GREEN, COLOR_CORE);


    if (telemetryDataRecord->main_door_opened)
        drawField("OPENED", 5, 212, 100, SCREEN_COLOR_LIGHT_RED, COLOR_CORE);
    else
        drawField("Closed", 5, 212, 100, SCREEN_GREEN, COLOR_CORE);

    tft->fillRect(5, 214, 97, 24, COLOR_CORE);
    tft->setCursor(5, 231);
    tft->setTextColor(SCREEN_GREEN);
    switch (telemetryDataRecord->heaterMode){
        case STAND_BY:
            tft->print("Stand By");
            break;
        case WARMING:
            tft->print("Warming");
            break;
        case FINAL_COOLING:
            tft->print("Cooling");
            break;
        case PID:
            tft->print("Burning");
            break;
        case OVERHEATED:
            tft->setTextColor(ST77XX_YELLOW);
            tft->print("Overheat");
            break;
        case CRITICAL:
            tft->setTextColor(SCREEN_COLOR_LIGHT_RED);
            tft->print("Critical");
            break;
    }

    drawFloatField("%0.2fC", telemetryDataRecord->accumulator_top_temp_sma, 130, 35, 80, SCREEN_GREEN, COLOR_ACCUMULATOR);
    drawFloatField("%0.2fC", telemetryDataRecord->accumulator_higher_temp_sma, 130, 75, 80, SCREEN_GREEN, COLOR_ACCUMULATOR);
    drawFloatField("%0.2fC", telemetryDataRecord->accumulator_lower_temp_sma, 130, 115, 80, SCREEN_GREEN, COLOR_ACCUMULATOR);
    drawFloatField("%0.2fC", telemetryDataRecord->accumulator_bottom_temp_sma, 130, 155, 80, SCREEN_GREEN, COLOR_ACCUMULATOR);

    drawPumpState(0, telemetryDataRecord->pump_1_state);
    drawPumpState(1, telemetryDataRecord->pump_2_state);

    drawFloatField("%0.2fC", telemetryDataRecord->internal_temp, 225, 175, 87, SCREEN_COLOR_GRAY, COLOR_CONTROLLER);
    drawIntField("%d", (int)telemetryDataRecord->pid_p, 225, 194, 87, SCREEN_COLOR_GRAY, COLOR_CONTROLLER);
    drawFloatField("%0.1f", telemetryDataRecord->pid_i, 225, 213, 87, SCREEN_COLOR_GRAY, COLOR_CONTROLLER);
    drawFloatField("%0.1f", telemetryDataRecord->pid_d, 225, 232, 87, SCREEN_COLOR_GRAY, COLOR_CONTROLLER);

}

void Display::drawPumpState(int index, bool is_on) {
    int outer_color, inner_color;
    if (is_on){
        outer_color = SCREEN_GREEN;
        inner_color = SCREEN_LIGHT_LIGHT_GREEN;
    } else {
        outer_color = ST77XX_RED;
        inner_color = SCREEN_LIGHT_LIGHT_RED;
    }
    tft->drawCircle(135 + index * 50, 200, 20, outer_color);
    tft->drawCircle(135 + index * 50, 200, 19, outer_color);
    tft->fillCircle(135 + index * 50, 200, 18, inner_color);
    tft->fillTriangle(142 + index * 50, 190, 122 + index * 50, 200, 142 + index * 50, 210, outer_color);


}

Adafruit_ST7789 *Display::getTft() {
    return tft;
}