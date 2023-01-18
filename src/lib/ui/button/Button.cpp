//
// Created by dsporykhin on 12.01.23.
//

#include <lib/ui/bufferedGraphics/DisplayBuffer.h>
#include <lib/adafruit/gfxfont.h>
#include <SPIFFS.h>
#include "lib/adafruit/Fonts/FreeSans12pt7b.h"
#include "Button.h"

Button::Button(ILI9488 *display, int x, int y, int width, int height,
               const char *icon_file_name_on, const char *icon_file_name_off, int icon_width, int icon_height,
               int bg_color) {


    this->icon_file_name_on = icon_file_name_on;
    this->icon_file_name_off = icon_file_name_off;
    this->icon_width = icon_width;
    this->icon_height = icon_height;

    this->x = x;
    this->y = y;
    this->width = width;
    this->height = height;

    this->bg_color = DisplayBuffer::color24To16(bg_color);
    this->display = display;

    need_redraw = true;
    state = false;
}

//float Button::getValue() {
//    return state;
//}
//
float Button::setState(bool state) {
    this->state = state;
    need_redraw = true;
}

void Button::draw() {
    if (!need_redraw)
        return;
    need_redraw = false;

    DisplayBuffer *displayBuffer = defaultDisplayBuffer ? defaultDisplayBuffer : new DisplayBuffer(width, height);
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

    int x_icon = (width - icon_width) >> 1;
    int y_icon = BUTTON_GRAPH_MARGIN_TOP + icon_height;
    int shift = y_icon * width + x_icon;

    Serial.println("free mem = " + String(esp_get_free_heap_size()));

    for (int row = height - 1; row >= 0; row--) {
        file.readBytes((char *) &displayBuffer->buffer[shift], icon_width * 2);
        shift -= width;
    }

    file.close();

    display->drawImage((uint8_t*)displayBuffer->buffer, x, y, width, height);
    if (!defaultDisplayBuffer) {
        displayBuffer->freeBuffer();
        free(displayBuffer);
    }

}
