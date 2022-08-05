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
    tft->init(240, 320, SPI_MODE2);
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

void Display::initScreen0() {
    tft->fillScreen(SCREEN_COLOR_BACKGROUND);
//    tft->fillScreen(SCREEN_COLOR_GRAY);
    tft->drawRect(1, 1, 105, 239, ST77XX_BLACK);
    tft->setFont(&FreeSans12pt7b);

    tft->setCursor(5, 20);

    tft->fillTriangle(82, 38, 102, 48, 82, 58, ST77XX_RED);

    tft->drawRect(105, 136, 20, 20, ST77XX_BLUE);
    tft->fillRect(106, 137, 18, 18, SCREEN_LIGHT_LIGHT_BLUE);

    tft->drawRect(105, 40, 20, 20, ST77XX_RED);
    tft->fillRect(106, 41, 18, 18, SCREEN_LIGHT_LIGHT_RED);

    tft->drawRoundRect(125, 10, 88, 160, 10, ST77XX_BLACK);

    tft->drawCircle(135, 200, 20, SCREEN_GREEN);
    tft->drawCircle(135, 200, 19, SCREEN_GREEN);
    tft->fillCircle(135, 200, 18, SCREEN_LIGHT_LIGHT_GREEN);
    tft->fillTriangle(142, 190, 122, 200, 142, 210, SCREEN_GREEN);
    tft->fillTriangle(100, 136, 81, 146, 100, 156, ST77XX_BLUE);

    tft->drawCircle(185, 200, 20, SCREEN_GREEN);
    tft->drawCircle(185, 200, 19, SCREEN_GREEN);
    tft->fillCircle(185, 200, 18, SCREEN_LIGHT_LIGHT_GREEN);
    tft->fillTriangle(192, 190, 172, 200, 192, 210, SCREEN_GREEN);

    tft->drawRoundRect(220, 154, 96, 85, 10, SCREEN_COLOR_LIGHT_GRAY);

    if (telemetry_initialized)
        drawScreen0(&savedDataRecord);
}

void Display::drawScreen0(TelemetryDataRecord *telemetryDataRecord) {
    char buffer[30];

    tft->setFont(&FreeSans12pt7b);

    tft->fillRect(5, 3, 97, 19, SCREEN_COLOR_BACKGROUND);
    tft->setCursor(5, 20);
    tft->setTextColor(SCREEN_COLOR_LIGHT_GRAY);
    sprintf(buffer, "%d%%", (int) telemetryDataRecord->smoke_door_position);
    tft->print(buffer);

    tft->fillRect(5, 28, 77, 19, SCREEN_COLOR_BACKGROUND);
    tft->setCursor(5, 45);
    tft->setTextColor(ST77XX_RED);
    sprintf(buffer, "%0.2fC", telemetryDataRecord->output_temp_sma);
    tft->print(buffer);

    tft->fillRect(5, 47, 77, 19, SCREEN_COLOR_BACKGROUND);
    tft->setCursor(5, 64);
    tft->setTextColor(ST77XX_RED);
    sprintf(buffer, "%d L/m", (int) telemetryDataRecord->core_flow);
    tft->print(buffer);

    tft->fillRect(5, 72, 77, 19, SCREEN_COLOR_BACKGROUND);
    tft->setCursor(5, 89);
    tft->setTextColor(SCREEN_COLOR_CYAN);
    sprintf(buffer, "%d%%", (int) telemetryDataRecord->upper_door_position);
    tft->print(buffer);

    tft->fillRect(5, 91, 77, 19, SCREEN_COLOR_BACKGROUND);
    tft->setCursor(5, 108);
    tft->setTextColor(SCREEN_COLOR_ORANGE);
    sprintf(buffer, "%0.2fC", telemetryDataRecord->core_temp_sma);
    tft->print(buffer);

    tft->fillRect(5, 110, 97, 19, SCREEN_COLOR_BACKGROUND);
    tft->setCursor(5, 127);
    tft->setTextColor(SCREEN_COLOR_ORANGE);
    sprintf(buffer, "%dW", (int) telemetryDataRecord->core_EMA_power);
    tft->print(buffer);

    tft->fillRect(5, 135, 77, 19, SCREEN_COLOR_BACKGROUND);
    tft->setCursor(5, 152);
    tft->setTextColor(ST77XX_BLUE);
    sprintf(buffer, "%0.2fC", telemetryDataRecord->input_temp_sma);
    tft->print(buffer);

    tft->fillRect(5, 154, 77, 19, SCREEN_COLOR_BACKGROUND);
    tft->setCursor(5, 171);
    tft->setTextColor(SCREEN_COLOR_CYAN);
    sprintf(buffer, "%d%%", (int) telemetryDataRecord->oxygen_door_position);
    tft->print(buffer);

    tft->fillRect(5, 176, 97, 19, SCREEN_COLOR_BACKGROUND);
    tft->setCursor(5, 193);
    tft->setTextColor(SCREEN_GREEN);
    if (telemetryDataRecord->time_to_close_oxygen_door_in_stanby_mode
        && (telemetryDataRecord->time_to_close_oxygen_door_in_stanby_mode > millis())) {
        int min = (telemetryDataRecord->time_to_close_oxygen_door_in_stanby_mode - millis()) / 1000;
        int sec = min % 60;
        min = min / 60;
        sprintf(buffer, "%2d:%2d", min, sec);
        tft->print(buffer);
    }

    tft->fillRect(5, 195, 100, 19, SCREEN_COLOR_BACKGROUND);
    tft->setCursor(5, 212);
    if (telemetryDataRecord->main_door_opened) {
        tft->setTextColor(SCREEN_COLOR_LIGHT_RED);
        tft->print("OPENED");
    } else {
        tft->setTextColor(SCREEN_GREEN);
        tft->print("Closed");
    }

    tft->fillRect(5, 214, 97, 24, SCREEN_COLOR_BACKGROUND);
    tft->setCursor(5, 231);
    tft->setTextColor(SCREEN_GREEN);
    switch (telemetryDataRecord->heaterMode){
        case STAND_BY:  tft->print("Stand By");
            break;
        case WARMING:  tft->print("Warming");
            break;
        case FINAL_COOLING:  tft->print("Cooling");
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


    tft->setTextColor(SCREEN_GREEN);
    tft->fillRect(130, 18, 80, 19, SCREEN_COLOR_BACKGROUND);
    tft->setCursor(130, 35);
    sprintf(buffer, "%0.2fC", telemetryDataRecord->accumulator_top_temp_sma);
    tft->print(buffer);

    tft->setTextColor(SCREEN_GREEN);
    tft->fillRect(130, 58, 80, 19, SCREEN_COLOR_BACKGROUND);
    tft->setCursor(130, 75);
    sprintf(buffer, "%0.2fC", telemetryDataRecord->accumulator_higher_temp_sma);
    tft->print(buffer);

    tft->setTextColor(SCREEN_GREEN);
    tft->fillRect(130, 98, 80, 19, SCREEN_COLOR_BACKGROUND);
    tft->setCursor(130, 115);
    sprintf(buffer, "%0.2fC", telemetryDataRecord->accumulator_lower_temp_sma);
    tft->print(buffer);

    tft->setTextColor(SCREEN_GREEN);
    tft->fillRect(130, 138, 80, 19, SCREEN_COLOR_BACKGROUND);
    tft->setCursor(130, 155);
    sprintf(buffer, "%0.2fC", telemetryDataRecord->accumulator_bottom_temp_sma);
    tft->print(buffer);

    drawPumpState(0, telemetryDataRecord->pump_1_state);
    drawPumpState(1, telemetryDataRecord->pump_2_state);

    tft->setTextColor(SCREEN_COLOR_LIGHT_GRAY);
    tft->fillRect(225, 158, 87, 19, SCREEN_COLOR_BACKGROUND);
    tft->setCursor(225, 175);
    sprintf(buffer, "%0.2fC", telemetryDataRecord->internal_temp);
    tft->print(buffer);

    tft->fillRect(225, 177, 87, 19, SCREEN_COLOR_BACKGROUND);
    tft->setCursor(225, 194);
    sprintf(buffer, "%d", (int)telemetryDataRecord->pid_p);
    tft->print(buffer);

    tft->fillRect(225, 196, 87, 19, SCREEN_COLOR_BACKGROUND);
    tft->setCursor(225, 213);
    sprintf(buffer, "%0.1f", telemetryDataRecord->pid_i);
    tft->print(buffer);

    tft->fillRect(225, 215, 87, 19, SCREEN_COLOR_BACKGROUND);
    tft->setCursor(225, 232);
    sprintf(buffer, "%0.1f", telemetryDataRecord->pid_d);
    tft->print(buffer);

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