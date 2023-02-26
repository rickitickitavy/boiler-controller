//
// Created by dsporykhin on 24.07.22.
//

#include <SPIFFS.h>
#include <lib/ui/bufferedGraphics/DisplayBuffer.h>
#include <lib/xpt2046/xpt2046.h>
#include "TouchDisplayController.h"
#include "Defines.h"
#include "lib/adafruit/Fonts/FreeSans12pt7b.h"
#include "lib/adafruit/Fonts/FreeSerif9pt7b.h"
#include "lib/adafruit/Fonts/FreeMono12pt7b.h"
#include "HeaterController.h"
#include "lib/ui/gauge/Gauge.h"

TouchDisplayController *TouchDisplayController::instance;

TouchDisplayController::TouchDisplayController() {
    instance = this;

    telemetry_initialized = false;
    tft = new ILI9488(DISPLAY_CS_PIN, DISPLAY_DC_PIN, DISPLAY_RST_PIN);
    tft->begin();
    tft->setRotation(1);
    tft->fillScreen(0);

    tft->setFont(&FreeSans12pt7b);

    tft->setCursor(0, 0);
    tft->setTextColor(0xff00, 0x00ff);
    tft->fillScreen(COLOR_BACKGROUND);

    touch = new XPT2046(TOUCH_CS, TOUCH_PEN);
    touch->begin(320, 480);  // Must be done before setting rotation
    touch->setCalibration(181, 249, 1840, 1800);
    touch->setRotation(touch->ROT90);

    _need_redraw = false;

    screen_touched = false;
    _buttons_count = 0;

    defaultDisplayBuffer = new DisplayBuffer(UI_PAGE_0_SIZE_GAUGE_WIDTH, UI_PAGE_0_SIZE_GAUGE_HEIGHT);
    // core temperature
    gauge_core_tempr = new Gauge(tft, "Core T (C)", 0, 0, UI_PAGE_0_SIZE_GAUGE_WIDTH, UI_PAGE_0_SIZE_GAUGE_HEIGHT,
                                 UI_PAGE_0_COLOR_GAUGE_BACKGROUND_NORMAL, 0xffffff, NULL, 2, 40, 103,
                                 new ColorPart(UI_COLOR_GAUGE_BLUE, 55,
                                               new ColorPart(UI_COLOR_GAUGE_GREEN, 90,
                                                             new ColorPart(
                                                                     UI_COLOR_GAUGE_YELLOW,
                                                                     96, new ColorPart(
                                                                             UI_COLOR_GAUGE_RED,
                                                                             0, NULL)))));
    gauge_core_tempr->defaultDisplayBuffer = defaultDisplayBuffer;

    // core power
    gauge_core_power = new Gauge(tft, "Core P(W)", (UI_PAGE_0_SIZE_GAUGE_WIDTH + 3), 0, UI_PAGE_0_SIZE_GAUGE_WIDTH,
                                 UI_PAGE_0_SIZE_GAUGE_HEIGHT,
                                 UI_PAGE_0_COLOR_GAUGE_BACKGROUND_NORMAL,
                                 0xffffff, NULL, 0, 3000, 45000, new ColorPart(UI_COLOR_GAUGE_BLUE, 15000,
                                                                               new ColorPart(UI_COLOR_GAUGE_GREEN,
                                                                                             35000,
                                                                                             new ColorPart(
                                                                                                     UI_COLOR_GAUGE_YELLOW,
                                                                                                     43000,
                                                                                                     new ColorPart(
                                                                                                             UI_COLOR_GAUGE_RED,
                                                                                                             0,
                                                                                                             NULL)))));
    gauge_core_power->defaultDisplayBuffer = defaultDisplayBuffer;

    // warming tempr
    gauge_warm_flow_tempr = new Gauge(tft, "Warm T(C)", (UI_PAGE_0_SIZE_GAUGE_WIDTH + 3) * 2, 0,
                                      UI_PAGE_0_SIZE_GAUGE_WIDTH, UI_PAGE_0_SIZE_GAUGE_HEIGHT,
                                      UI_PAGE_0_COLOR_GAUGE_BACKGROUND_NORMAL,
                                      0xffffff, NULL, 2, 30, 90, new ColorPart(UI_COLOR_GAUGE_BLUE, 50,
                                                                               new ColorPart(UI_COLOR_GAUGE_GREEN, 75,
                                                                                             new ColorPart(
                                                                                                     UI_COLOR_GAUGE_YELLOW,
                                                                                                     83, new ColorPart(
                                                                                                             UI_COLOR_GAUGE_RED,
                                                                                                             0,
                                                                                                             NULL)))));
    gauge_warm_flow_tempr->defaultDisplayBuffer = defaultDisplayBuffer;

    // core output flow tempr
    gauge_core_output_tempr = new Gauge(tft, "Output T", (UI_PAGE_0_SIZE_GAUGE_WIDTH + 3) * 0,
                                        (UI_PAGE_0_SIZE_GAUGE_HEIGHT + 3) * 1, UI_PAGE_0_SIZE_GAUGE_WIDTH,
                                        UI_PAGE_0_SIZE_GAUGE_HEIGHT,
                                        UI_PAGE_0_COLOR_GAUGE_BACKGROUND_NORMAL,
                                        0xffffff, NULL, 2, 40, 105, new ColorPart(UI_COLOR_GAUGE_BLUE, 60,
                                                                                  new ColorPart(UI_COLOR_GAUGE_GREEN,
                                                                                                87,
                                                                                                new ColorPart(
                                                                                                        UI_COLOR_GAUGE_YELLOW,
                                                                                                        96,
                                                                                                        new ColorPart(
                                                                                                                UI_COLOR_GAUGE_RED,
                                                                                                                0,
                                                                                                                NULL)))));
    gauge_core_output_tempr->defaultDisplayBuffer = defaultDisplayBuffer;

    // power balance
    gauge_power_balance = new Gauge(tft, "Pwr P(kW)", (UI_PAGE_0_SIZE_GAUGE_WIDTH + 3) * 1,
                                    (UI_PAGE_0_SIZE_GAUGE_HEIGHT + 3) * 1, UI_PAGE_0_SIZE_GAUGE_WIDTH,
                                    UI_PAGE_0_SIZE_GAUGE_HEIGHT,
                                    UI_PAGE_0_COLOR_GAUGE_BACKGROUND_NORMAL,
                                    0xffffff, NULL, 1, -20, 40, new ColorPart(UI_COLOR_GAUGE_BLUE, 0,
                                                                              new ColorPart(UI_COLOR_GAUGE_RED, 0,
                                                                                            NULL)));
    gauge_power_balance->defaultDisplayBuffer = defaultDisplayBuffer;


    // acc top
    gauge_acc_top_tempr = new Gauge(tft, "Top T(C)", (UI_PAGE_0_SIZE_GAUGE_WIDTH + 3) * 2,
                                    (UI_PAGE_0_SIZE_GAUGE_HEIGHT + 3) * 1, UI_PAGE_0_SIZE_GAUGE_WIDTH,
                                    UI_PAGE_0_SIZE_GAUGE_HEIGHT,
                                    UI_PAGE_0_COLOR_GAUGE_BACKGROUND_NORMAL,
                                    0xffffff, NULL, 2, 30, 90, new ColorPart(UI_COLOR_GAUGE_BLUE, 45,
                                                                             new ColorPart(UI_COLOR_GAUGE_GREEN, 75,
                                                                                           new ColorPart(
                                                                                                   UI_COLOR_GAUGE_YELLOW,
                                                                                                   80, new ColorPart(
                                                                                                           UI_COLOR_GAUGE_RED,
                                                                                                           0, NULL)))));
    gauge_acc_top_tempr->defaultDisplayBuffer = defaultDisplayBuffer;

    // input flow tempr
    gauge_core_input_tempr = new Gauge(tft, "Input T", (UI_PAGE_0_SIZE_GAUGE_WIDTH + 3) * 0,
                                       (UI_PAGE_0_SIZE_GAUGE_HEIGHT + 3) * 2, UI_PAGE_0_SIZE_GAUGE_WIDTH,
                                       UI_PAGE_0_SIZE_GAUGE_HEIGHT,
                                       UI_PAGE_0_COLOR_GAUGE_BACKGROUND_NORMAL,
                                       0xffffff, NULL, 2, 30, 85, new ColorPart(UI_COLOR_GAUGE_BLUE, 50,
                                                                                new ColorPart(UI_COLOR_GAUGE_GREEN, 75,
                                                                                              new ColorPart(
                                                                                                      UI_COLOR_GAUGE_YELLOW,
                                                                                                      83, new ColorPart(
                                                                                                              UI_COLOR_GAUGE_RED,
                                                                                                              0,
                                                                                                              NULL)))));
    gauge_core_input_tempr->defaultDisplayBuffer = defaultDisplayBuffer;

    // accumulated power
    gauge_accumulated_energy = new Gauge(tft, "Energy kWh", (UI_PAGE_0_SIZE_GAUGE_WIDTH + 3) * 1,
                            (UI_PAGE_0_SIZE_GAUGE_HEIGHT + 3) * 2, UI_PAGE_0_SIZE_GAUGE_WIDTH,
                            UI_PAGE_0_SIZE_GAUGE_HEIGHT,
                            UI_PAGE_0_COLOR_GAUGE_BACKGROUND_NORMAL,
                            0xffffff, NULL, 2, -15, 70, new ColorPart(UI_COLOR_GAUGE_BLUE, 0,
                                                                      new ColorPart(UI_COLOR_GAUGE_GREEN, 56,
                                                                                    new ColorPart(UI_COLOR_GAUGE_YELLOW,
                                                                                                  65, new ColorPart(
                                                                                                    UI_COLOR_GAUGE_RED,
                                                                                                    0, NULL)))));
    gauge_accumulated_energy->defaultDisplayBuffer = defaultDisplayBuffer;

    // acc bottom
    gauge_acc_bottom_tempr = new Gauge(tft, "Bottom T", (UI_PAGE_0_SIZE_GAUGE_WIDTH + 3) * 2,
                                       (UI_PAGE_0_SIZE_GAUGE_HEIGHT + 3) * 2, UI_PAGE_0_SIZE_GAUGE_WIDTH,
                                       UI_PAGE_0_SIZE_GAUGE_HEIGHT,
                                       UI_PAGE_0_COLOR_GAUGE_BACKGROUND_NORMAL,
                                       0xffffff, NULL, 2, 30, 80, new ColorPart(UI_COLOR_GAUGE_BLUE, 35,
                                                                                new ColorPart(UI_COLOR_GAUGE_GREEN, 68,
                                                                                              new ColorPart(
                                                                                                      UI_COLOR_GAUGE_YELLOW,
                                                                                                      75, new ColorPart(
                                                                                                              UI_COLOR_GAUGE_RED,
                                                                                                              0,
                                                                                                              NULL)))));
    gauge_acc_bottom_tempr->defaultDisplayBuffer = defaultDisplayBuffer;

    button_pumps = new Button(tft,
                              378,
                              (UI_PAGE_0_SIZE_GAUGE_HEIGHT + 3) * 0,
                              100, UI_PAGE_0_SIZE_GAUGE_HEIGHT,
                              "/img/pumps_on.bmp", "/img/pumps_off.bmp", 86, 62,
                              UI_PAGE_0_COLOR_GAUGE_BACKGROUND_NORMAL);

    button_pumps->defaultDisplayBuffer = defaultDisplayBuffer;

    button_init_fire = new Button(tft,
                                  378,
                                  (UI_PAGE_0_SIZE_GAUGE_HEIGHT + 3) * 1,
                                  100, UI_PAGE_0_SIZE_GAUGE_HEIGHT,
                                  "/img/init_fire_on.bmp", "/img/init_fire_off.bmp", 74, 77,
                                  UI_PAGE_0_COLOR_GAUGE_BACKGROUND_NORMAL);

    button_init_fire->defaultDisplayBuffer = defaultDisplayBuffer;
    button_init_fire->action = initFireButtonAction;
    button_init_fire->drawAction = initFireButtonDrawAction;
    addButton(button_init_fire);
    screen_index = -1;
}

void TouchDisplayController::setHeaterController(HeaterController *heaterController) {
    this->heaterController = heaterController;
}

void TouchDisplayController::initFireButtonDrawAction(DisplayBuffer *canvas){
    HeaterController *heaterController = instance->heaterController;
    if (heaterController->isOxygenDoorOpenedForATime()){

        int est = heaterController->getOxygenDoorOpenedForATime();
        int prcnt = est * 100 / TIME_FOR_INIT_FIRE_SEC;
        float theta = PI * (float)est / (float)TIME_FOR_INIT_FIRE_SEC;

        Serial.println("theta = " + String(theta));

        int _color;
        if (prcnt > 33)
            _color = UI_PAGE_0_COLOR_GAUGE_INITFIRE_EST_MANY;
        else if (prcnt > 10)
            _color = UI_PAGE_0_COLOR_GAUGE_INITFIRE_EST_LOW;
        else
            _color = UI_PAGE_0_COLOR_GAUGE_INITFIRE_EST_VERY_LOW;

        uint16_t color = canvas->color24To16(_color);
        uint16_t color_spent = canvas->color24To16(UI_PAGE_0_COLOR_GAUGE_INITFIRE_SPENT);
        for (int i = 0; i < 7; i++) {
            canvas->drawArc(50, UI_PAGE_0_SIZE_GAUGE_HEIGHT >> 1, 48 - i, 0.0, theta, 350, color);

            canvas->drawArc(50, UI_PAGE_0_SIZE_GAUGE_HEIGHT >> 1, 48 - i, theta, PI - theta, 350,
                            color_spent);
        }
    }
}

void TouchDisplayController::initFireButtonAction(DisplayButtonEvent event) {
    if (event == DisplayButtonEvent::DOWN) {
        HeaterController *heaterController = instance->heaterController;
        if (((heaterController->getTelemetryRecord()->heaterMode == STAND_BY))
            || (heaterController->getTelemetryRecord()->heaterMode == FINAL_COOLING)) {
            if (heaterController->isOxygenDoorOpenedForATime()) {
                heaterController->closeOxygenDoor();
                instance->button_init_fire->setState(false);
            } else {
                heaterController->openOxygenDoorForTime(TIME_FOR_INIT_FIRE_SEC);
                instance->button_init_fire->setState(true);
            }
            instance->dirty();
        }
    }
}

void TouchDisplayController::printStatus(const char *status) {
    tft->setTextColor(SCREEN_COLOR_LIGHT_RED);
    tft->println(status);
}

void TouchDisplayController::updateInfo(TelemetryDataRecord *telemetryDataRecord) {
    memcpy(&this->savedDataRecord, telemetryDataRecord, sizeof(TelemetryDataRecord));
    telemetry_initialized = true;
    drawScreen();
}

void TouchDisplayController::drawScreen() {
    _need_redraw = false;
    switch (screen_index) {
        case 0:
            drawScreen0(&savedDataRecord);
            break;
    }
}

void TouchDisplayController::setScreenIndex(int index) {
    if (screen_index != index) {
        screen_index = index;
        switch (screen_index) {
            case 0:
                initScreen0();
                break;
        }
    }
}

void
TouchDisplayController::drawFloatField(const char *msg, float value, int txt_x, int txt_y, int width,
                                       int font_color,
                                       int bg_color) {
    char buffer[32];
    sprintf(buffer, msg, value);
    drawField(buffer, txt_x, txt_y, width, font_color, bg_color);
}

void
TouchDisplayController::drawIntField(const char *msg, int value, int txt_x, int txt_y, int width, int font_color,
                                     int bg_color) {
    char buffer[32];
    sprintf(buffer, msg, value);
    drawField(buffer, txt_x, txt_y, width, font_color, bg_color);
}

void
TouchDisplayController::drawField(const char *msg, int txt_x, int txt_y, int width, int font_color, int bg_color) {
    tft->fillRect(txt_x, txt_y - 17, width, 19, bg_color);
    tft->setCursor(txt_x, txt_y);
    tft->setTextColor(font_color);
    tft->print(msg);
}

void TouchDisplayController::drawGauges() {
    handle();
    gauge_core_tempr->draw();
    handle();
    gauge_acc_bottom_tempr->draw();
    handle();
    gauge_warm_flow_tempr->draw();
    handle();
    gauge_core_input_tempr->draw();
    handle();
    gauge_power_balance->draw();
    handle();
    gauge_core_output_tempr->draw();
    handle();
    gauge_acc_top_tempr->draw();
    handle();
    gauge_core_power->draw();
    handle();
    gauge_accumulated_energy->draw();
    handle();
    uint16_t wdt = defaultDisplayBuffer->width();
    defaultDisplayBuffer->_setWHOnly(100, defaultDisplayBuffer->height());
    button_pumps->draw();
    handle();
    button_init_fire->draw();
    defaultDisplayBuffer->_setWHOnly(wdt, defaultDisplayBuffer->height());
    handle();
}

void TouchDisplayController::initScreen0() {
    tft->fillScreen(UI_PAGE_0_COLOR_MAIN_BACKGROUND);

    drawGauges();

    if (telemetry_initialized)
        drawScreen0(&savedDataRecord);
}

uint8_t *TouchDisplayController::loadImage(const char *file_name, uint16_t width, uint16_t height) {
    uint8_t *buffer;
    File file = SPIFFS.open(file_name, "r");
    if (!file)
        Serial.println("Failed to open the file");
    else {
        buffer = (uint8_t *) malloc(width * height * 2);

        if (!buffer) {
            Serial.print("Buffer allocation error for file ");
            Serial.println(file_name);

        } else
            for (int row = height - 1; row >= 0; row--) {
                file.readBytes((char *) &buffer[row * 2 * width], width * 2);
            }
    }
    file.close();
    return buffer;
}

void TouchDisplayController::drawScreen1(TelemetryDataRecord *telemetryDataRecord) {
    char buffer[30];

    tft->setFont(&FreeSans12pt7b);

    drawIntField("%d%%", (int) telemetryDataRecord->smoke_door_position, 5, 20, 97, SCREEN_COLOR_GRAY, COLOR_CORE);
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

void TouchDisplayController::drawScreen0(TelemetryDataRecord *telemetryDataRecord) {
    gauge_core_tempr->setValue(telemetryDataRecord->core_temp_sma);
    gauge_core_tempr->setBgColor(telemetryDataRecord->sensor_data_ready[T_SENS_INDEX_CORE]
                                 ? UI_PAGE_0_COLOR_GAUGE_BACKGROUND_NORMAL
                                 : UI_PAGE_0_COLOR_GAUGE_BACKGROUND_ERROR);

    gauge_core_power->setValue(telemetryDataRecord->core_EMA_power);
    gauge_warm_flow_tempr->setValue(telemetryDataRecord->forwar_flow_temp_sma);
    gauge_warm_flow_tempr->setBgColor(telemetryDataRecord->sensor_data_ready[T_SENS_INDEX_FORWARD_FLOW]
                                      ? UI_PAGE_0_COLOR_GAUGE_BACKGROUND_NORMAL
                                      : UI_PAGE_0_COLOR_GAUGE_BACKGROUND_ERROR);

    gauge_core_output_tempr->setValue(telemetryDataRecord->output_temp_sma);
    gauge_core_output_tempr->setBgColor(telemetryDataRecord->sensor_data_ready[T_SENS_INDEX_OUTPUT_FLOW]
                                        ? UI_PAGE_0_COLOR_GAUGE_BACKGROUND_NORMAL
                                        : UI_PAGE_0_COLOR_GAUGE_BACKGROUND_ERROR);
    if (telemetryDataRecord->power_balance_ready) {
        gauge_power_balance->setValue(telemetryDataRecord->power_balance_kwt_hour);
        gauge_power_balance->setBgColor(telemetryDataRecord->sensor_data_ready[T_SENS_INDEX_ACC_MID_HI]
                                            ? UI_PAGE_0_COLOR_GAUGE_BACKGROUND_NORMAL
                                            : UI_PAGE_0_COLOR_GAUGE_BACKGROUND_ERROR);

    }

    gauge_acc_top_tempr->setValue(telemetryDataRecord->accumulator_top_temp_sma);
    gauge_acc_top_tempr->setBgColor(telemetryDataRecord->sensor_data_ready[T_SENS_INDEX_ACC_TOP]
                                        ? UI_PAGE_0_COLOR_GAUGE_BACKGROUND_NORMAL
                                        : UI_PAGE_0_COLOR_GAUGE_BACKGROUND_ERROR);

    gauge_core_input_tempr->setValue(telemetryDataRecord->input_temp_sma);
    gauge_core_input_tempr->setBgColor(telemetryDataRecord->sensor_data_ready[T_SENS_INDEX_INPUT_FLOW]
                                    ? UI_PAGE_0_COLOR_GAUGE_BACKGROUND_NORMAL
                                    : UI_PAGE_0_COLOR_GAUGE_BACKGROUND_ERROR);

    gauge_accumulated_energy->setValue(telemetryDataRecord->accumulated_energy_kwt_hour);
    gauge_accumulated_energy->setBgColor(telemetryDataRecord->sensor_data_ready[T_SENS_INDEX_ACC_MID_LO]
                                    ? UI_PAGE_0_COLOR_GAUGE_BACKGROUND_NORMAL
                                    : UI_PAGE_0_COLOR_GAUGE_BACKGROUND_ERROR);

    gauge_acc_bottom_tempr->setValue(telemetryDataRecord->accumulator_bottom_temp_sma);
    gauge_acc_bottom_tempr->setBgColor(telemetryDataRecord->sensor_data_ready[T_SENS_INDEX_ACC_BOTTOM]
                                       ? UI_PAGE_0_COLOR_GAUGE_BACKGROUND_NORMAL
                                       : UI_PAGE_0_COLOR_GAUGE_BACKGROUND_ERROR);

    int btn_color = telemetryDataRecord->main_door_opened
                    ? UI_PAGE_0_COLOR_GAUGE_BACKGROUND_OPENED
                    : UI_PAGE_0_COLOR_GAUGE_BACKGROUND_NORMAL;

    button_pumps->setState(telemetryDataRecord->pump_1_state || telemetryDataRecord->pump_2_state);
    button_pumps->setBgColor(btn_color);

    button_init_fire->setState(heaterController->isOxygenDoorOpenedForATime());
    button_init_fire->setBgColor(btn_color);

    drawGauges();

}

ILI9488 *TouchDisplayController::getTft() {
    return tft;
}

bool TouchDisplayController::read_x_y(uint16_t &x, uint16_t &y) {
    uint16_t succ_count = 0;
    uint16_t _x = 0;
    uint16_t _y = 0;

    for (int i = 0; i < BUTTON_DETECT_X_Y_COUNT; i++) {
        if (touch->isTouching()) {
            uint16_t _x_t, _y_t;
            touch->getPosition(_x_t, _y_t);
            if ((_x_t <= 480) && (_y_t <=320)) {
                _x += _x_t;
                _y += _y_t;
                succ_count++;
            }
        }
    }
    if (succ_count > 0) {
        x = _x / succ_count;
        y = _y / succ_count;
    }

    bool succ = ((float) succ_count / (float) BUTTON_DETECT_X_Y_COUNT > 0.7);
    return succ;
}

void TouchDisplayController::addButton(Button *button) {
    if (_buttons_count < BUTTONS_MAX_COUNT) {
        _buttons[_buttons_count++] = button;
    }
}

void TouchDisplayController::handleTouchAction(DisplayButtonEvent event) {
    for (int button_index = 0; button_index < _buttons_count; button_index++) {
        uint16_t b_x, b_y, b_w, b_h;
        _buttons[button_index]->getXY(b_x, b_y);
        _buttons[button_index]->getWH(b_w, b_h);
        if ((_touch_x >= b_x) && (_touch_x <= (b_x + b_w)) && (_touch_y >= b_y) && (_touch_y <= (b_y + b_h)))
            if (_buttons[button_index]->action) {
                _buttons[button_index]->action(event);
            }
    }
}

void TouchDisplayController::handle() {
    if (touch->isTouching()) {
        // touch screen was touched
        if (!screen_touched) {
            if ((millis() - last_touch_state_changed_at) > BUTTON_DELAY_ANTI_BUZZLE) {
                // first touch
                screen_touched = read_x_y(_touch_x, _touch_y);
                if (screen_touched)
                    handleTouchAction(DisplayButtonEvent::DOWN);
                last_touch_state_changed_at = millis();
            }
        }
    } else {
        // touch screen is not touched
        if (screen_touched) {
            // touch screen was released
            screen_touched = false;
            last_touch_state_changed_at = millis();
            handleTouchAction(DisplayButtonEvent::UP);
        }
    }
}

void TouchDisplayController::dirty() {
    _need_redraw = true;
}

bool TouchDisplayController::isDirty() {
    return _need_redraw;
}
