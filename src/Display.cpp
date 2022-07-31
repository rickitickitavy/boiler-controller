//
// Created by dsporykhin on 24.07.22.
//

#include "Display.h"
#include "Defines.h"
#include "lib/adafruit/Fonts/FreeSans12pt7b.h"
#include "lib/adafruit/Fonts/FreeSerif9pt7b.h"
#include "lib/adafruit/Fonts/FreeMono12pt7b.h"

Display::Display() {
    tft = new Adafruit_ST7789(DISPLAY_CS_PIN, DISPLAY_DC_PIN, DISPLAY_RST_PIN);
    tft->init(240, 320, SPI_MODE2);
    tft->setRotation(3);
    tft->fillScreen(0);

//    tft->setFont(&FreeMonoBoldOblique18pt7b);
//    tft->setFont(&FreeSans12pt7b);
//    tft->setFont(&FreeSerif9pt7b);
    tft->setFont(&FreeMono12pt7b);

    tft->setCursor(20, 120);
    tft->setTextColor(0xff00, 0x00ff);

    for (int i = 0; i < SCREEN_0_DATA_LENGTH; i++) {
        screen_0_data[i].name[0] = 0;
        screen_0_data[i].value[0] = 0;
    }
}

void Display::printStatus(const char *status) {
    tft->fillRect(0, 80, 320, 80, 0);
    tft->setCursor(20, 120);
    tft->setTextColor(0xf800, 0x001f);
    tft->print(status);
}

void Display::drawScreen() {
    switch (screen_index) {
        case 0:
            drawScreen0();
            break;
    }
}

void Display::setScreenIndex(int index) {
    if (screen_index != index) {
        screen_index = index;
        drawScreen();
    }
}

void Display::drawScreen0() {
    tft->fillScreen(0xffff);
    tft->drawRect(0, 0, 320, 240, 0xBDF7);

//    tft->setTextColor(0x39e7);
    tft->setTextColor(0);

    for (int i = 0; i < SCREEN_0_DATA_LENGTH; i++) {
        int y = i * 24 + 22;
        int y_txt = y - 3;

        tft->setCursor(3, y_txt);
        tft->print(screen_0_data[i].name);

        tft->setCursor(FIRST_COLUMN_WIDTH + 4, y_txt);
        tft->print(screen_0_data[i].value);

        tft->drawLine(1, y, 318, y, 0xce79);
    }

    tft->drawLine(FIRST_COLUMN_WIDTH, 2, FIRST_COLUMN_WIDTH, 238, SCREEN_COLOR_LIGHT_GRAY);

    if (screen_0_data[0].value[0] == 'O'){
        tft->fillRect(FIRST_COLUMN_WIDTH + 1, 1, 238, 21, ST77XX_RED);
        tft->setCursor(FIRST_COLUMN_WIDTH + 4, 17);
        tft->setTextColor(ST77XX_YELLOW);
        tft->print(screen_0_data[0].value);
    }
}

void Display::setScreen0Parameter(int index, const char *name, const char *value) {
    if ((index >= 0) && (index < SCREEN_0_DATA_LENGTH)) {
        int name_len = strlen(name);
        if (name_len > SCREEN_0_MAX_NAME_LENGTH)
            name_len = SCREEN_0_MAX_NAME_LENGTH;
        memcpy(&screen_0_data[index].name, name, name_len);
        screen_0_data[index].name[name_len] = 0;

        int val_len = strlen(value);
        if (val_len > 6)
            val_len = 6;
        memcpy(&screen_0_data[index].value, value, val_len);
        screen_0_data[index].value[val_len] = 0;
    }
}

