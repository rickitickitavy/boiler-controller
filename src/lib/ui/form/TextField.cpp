#include <string.h>
#include "TextField.h"

TextField::TextField(ILI9488 *display, int x, int y, int width, int height)
        : display(display), x(x), y(y), width(width), height(height),
          focused(false), need_redraw(true) {
    value[0] = 0;
}

void TextField::setValue(const char *v) {
    if (!v) v = "";
    int n = 0;
    while (n < FORM_MAX_VALUE_LEN - 1) {
        unsigned char c = (unsigned char) v[n];
        if (c == 0 || c < 32 || c > 126)
            break;
        value[n] = (char) c;
        n++;
    }
    value[n] = 0;
    need_redraw = true;
}

const char *TextField::getValue() const { return value; }

void TextField::setFocused(bool f) {
    if (focused == f) return;
    focused = f;
    need_redraw = true;
}

bool TextField::isFocused() const { return focused; }

void TextField::setPosition(int nx, int ny) {
    x = nx;
    y = ny;
    need_redraw = true;
}

void TextField::setWidth(int nw) {
    if (nw < 1) nw = 1;
    width = nw;
    need_redraw = true;
}

bool TextField::hitTest(uint16_t tx, uint16_t ty) const {
    return tx >= (uint16_t) x && tx < (uint16_t) (x + width)
           && ty >= (uint16_t) y && ty < (uint16_t) (y + height);
}

void TextField::invalidate() { need_redraw = true; }

void TextField::draw() {
    if (!need_redraw) return;
    need_redraw = false;
    const uint16_t bg = focused ? FORM_COLOR_BG_FOCUSED : FORM_COLOR_BG_UNFOCUSED;
    display->fillRect(x, y, width, height, bg);
    display->drawRect(x, y, width, height, FORM_COLOR_BORDER);
    display->setFont(FORM_FONT);
    display->setTextSize(1);
    display->setTextColor(FORM_COLOR_TEXT);
    display->setCursor(x + FORM_TEXT_PAD_X, y + FORM_TEXT_BASELINE);
    display->print(value);
}

void TextField::getXY(uint16_t &ox, uint16_t &oy) const { ox = x; oy = y; }
void TextField::getWH(uint16_t &w, uint16_t &h) const { w = width; h = height; }
