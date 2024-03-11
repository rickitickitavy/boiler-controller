//
// Created by dsporykhin on 12.01.23.
//

#include <lib/ui/bufferedGraphics/DisplayBuffer.h>
#include <SPIFFS.h>
#include "lib/adafruit/Fonts/FreeSans12pt7b.h"
#include "Button.h"
#include <sys/param.h>

Button::Button(ILI9488 *display, int x, int y, int width, int height,
               const char *icon_file_name_on, const char *icon_file_name_off, int icon_width, int icon_height,
               int bg_color) {

    this->icon_file_name_on = icon_file_name_on;
    this->icon_file_name_off = icon_file_name_off;
    this->icon_width = icon_width;
    this->icon_height = icon_height;

    this->x = x;
    this->y = y;
    this->_width = width;
    this->_height = height;

    this->bg_color = DisplayBuffer::color24To16(bg_color);
    this->display = display;

    need_redraw = true;
    state = false;
}

float Button::setState(bool state) {
    this->state = state;
    need_redraw = true;
}

void Button::setCaption(const char* new_caption){

    if (!caption)
        caption = (char *)malloc(BUTTON_MAX_CAPTION_LENGTH + 1);

    int new_caption_len = MIN(strlen(new_caption), BUTTON_MAX_CAPTION_LENGTH);
    caption[new_caption_len] = 0;
    memcpy(caption, new_caption, new_caption_len);

}

void Button::draw() {
    if (!need_redraw)
        return;
    need_redraw = false;

    DisplayBuffer *displayBuffer = defaultDisplayBuffer ? defaultDisplayBuffer : new DisplayBuffer(_width, _height);
    displayBuffer->fillScreen(bg_color);


    File file;
    if (state)
        file = SPIFFS.open(icon_file_name_on, "r");
    else
        file = SPIFFS.open(icon_file_name_off, "r");


    if (!file) {
        Serial.println("Failed to open the file");
        displayBuffer->freeBuffer();
        free(displayBuffer);
        return;
    }

    int x_icon = (_width - icon_width) >> 1;
    int y_icon = BUTTON_GRAPH_MARGIN_TOP + icon_height;
    int shift = y_icon * _width + x_icon;

    for (int row = _height - 1; row >= 0; row--) {
        file.readBytes((char *) &displayBuffer->buffer[shift], icon_width * 2);
        shift -= _width;
    }

    file.close();

    if (drawAction)
        drawAction(displayBuffer);

    if (caption){
        displayBuffer->setCursor(5, _height - 8);
        displayBuffer->write(caption);
    }

    display->drawImage((uint8_t*)displayBuffer->buffer, x, y, _width, _height);
    if (!defaultDisplayBuffer) {
        displayBuffer->freeBuffer();
        free(displayBuffer);
    }

}

void Button::getXY(uint16_t &x, uint16_t &y) {
    x = this->x;
    y = this->y;
}

void Button::getWH(uint16_t &w, uint16_t &h) {
    w = _width;
    h = _height;
}

void Button::setBgColor(int color) {
    bg_color = DisplayBuffer::color24To16(color);
}


