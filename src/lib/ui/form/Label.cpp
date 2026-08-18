#include <string.h>
#include "Label.h"

Label::Label(ILI9488 *display, int x, int y, int width, int height)
        : display(display), x(x), y(y), width(width), height(height),
          fg_color(FORM_COLOR_TEXT), bg_color(FORM_COLOR_LABEL_BG), need_redraw(true) {
    text[0] = 0;
}

void Label::setText(const char *value) {
    if (!value)
        value = "";
    strncpy(text, value, FORM_MAX_VALUE_LEN - 1);
    text[FORM_MAX_VALUE_LEN - 1] = 0;
    need_redraw = true;
}

const char *Label::getText() const { return text; }

void Label::setPosition(int nx, int ny) {
    x = nx;
    y = ny;
    need_redraw = true;
}

void Label::setWidth(int nw) {
    if (nw < 1) nw = 1;
    width = nw;
    need_redraw = true;
}

void Label::setColors(uint16_t fg, uint16_t bg) {
    fg_color = fg;
    bg_color = bg;
    need_redraw = true;
}

void Label::invalidate() { need_redraw = true; }

void Label::draw() {
    if (!need_redraw)
        return;
    need_redraw = false;
    display->fillRect(x, y, width, height, bg_color);
    display->setFont(FORM_LABEL_FONT);
    display->setTextSize(1);
    display->setTextColor(fg_color);
    display->setCursor(x + FORM_TEXT_PAD_X, y + FORM_LABEL_BASELINE);
    display->print(text);
}

void Label::getXY(uint16_t &ox, uint16_t &oy) const { ox = x; oy = y; }
void Label::getWH(uint16_t &w, uint16_t &h) const { w = width; h = height; }
