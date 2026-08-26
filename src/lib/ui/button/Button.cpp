//
// Created by dsporykhin on 12.01.23.
//

#include <lib/ui/bufferedGraphics/DisplayBuffer.h>
#include <LittleFS.h>
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
    return state ? 1.0f : 0.0f;
}

void Button::setCaption(const char* new_caption){

    if (!caption)
        caption = (char *)malloc(BUTTON_MAX_CAPTION_LENGTH + 1);

    int new_caption_len = MIN(strlen(new_caption), BUTTON_MAX_CAPTION_LENGTH);
    caption[new_caption_len] = 0;
    memcpy(caption, new_caption, new_caption_len);
    need_redraw = true;

}

void Button::draw() {
    if (!need_redraw)
        return;
    need_redraw = false;

    const bool ownsBuffer = (defaultDisplayBuffer == nullptr);
    DisplayBuffer *displayBuffer = ownsBuffer ? new DisplayBuffer(_width, _height) : defaultDisplayBuffer;
    displayBuffer->fillScreen(bg_color);


    File file;
    if (state)
        file = LittleFS.open(icon_file_name_on, "r");
    else
        file = LittleFS.open(icon_file_name_off, "r");


    if (!file) {
        Serial.println("Failed to open the file");
        // Never free shared defaultDisplayBuffer — that caused heap corruption after FD/open failures.
        if (ownsBuffer) {
            displayBuffer->freeBuffer();
            delete displayBuffer;
        }
        return;
    }

    int x_icon = (_width - icon_width) >> 1;
    if (x_icon < 0)
        x_icon = 0;
    const int y_top = BUTTON_GRAPH_MARGIN_TOP;
    const int bufPixels = displayBuffer->width() * displayBuffer->height();

    // BMP payload is bottom-up; blit only icon_height rows into the button content area.
    for (int fileRow = 0; fileRow < icon_height; fileRow++) {
        const int dest_y = y_top + (icon_height - 1 - fileRow);
        if (dest_y < 0 || dest_y >= _height) {
            char discard[160];
            int remaining = icon_width * 2;
            while (remaining > 0) {
                const int chunk = remaining > (int) sizeof(discard) ? (int) sizeof(discard) : remaining;
                file.readBytes(discard, chunk);
                remaining -= chunk;
            }
            continue;
        }
        const int shift = dest_y * _width + x_icon;
        if (shift < 0 || shift + icon_width > bufPixels)
            break;
        file.readBytes((char *) &displayBuffer->buffer[shift], icon_width * 2);
    }

    file.close();

    if (drawAction)
        drawAction(displayBuffer);

    if (caption){
        displayBuffer->setCursor(5, _height - 8);
        displayBuffer->write(caption);
    }

    display->drawImage((uint8_t*)displayBuffer->buffer, x, y, _width, _height);
    if (ownsBuffer) {
        displayBuffer->freeBuffer();
        delete displayBuffer;
    }

}

void Button::invalidate() {
    need_redraw = true;
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
    uint16_t next = DisplayBuffer::color24To16(color);
    if (bg_color != next) {
        bg_color = next;
        need_redraw = true;
    }
}

