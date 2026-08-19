#include <string.h>
#include "DropDownList.h"

DropDownList::DropDownList(ILI9488 *display, int x, int y, int width, int height)
        : display(display), x(x), y(y), width(width), height(height),
          option_count(0), selected_index(0), open(false), focused(false),
          need_redraw(true), list_need_redraw(true), list_scroll(0),
          list_max_height(FORM_DROPDOWN_ROW_HEIGHT * 5) {
}

void DropDownList::clearOptions() {
    option_count = 0;
    selected_index = 0;
    need_redraw = true;
    list_need_redraw = true;
}

bool DropDownList::addOption(const char *value) {
    if (option_count >= FORM_DROPDOWN_MAX_OPTIONS || !value)
        return false;
    strncpy(options[option_count], value, FORM_MAX_VALUE_LEN - 1);
    options[option_count][FORM_MAX_VALUE_LEN - 1] = 0;
    option_count++;
    need_redraw = true;
    list_need_redraw = true;
    return true;
}

void DropDownList::setSelectedIndex(int index) {
    if (option_count <= 0) {
        selected_index = 0;
        return;
    }
    if (index < 0) index = 0;
    if (index >= option_count) index = option_count - 1;
    selected_index = index;
    need_redraw = true;
}

int DropDownList::getSelectedIndex() const { return selected_index; }

const char *DropDownList::getSelectedValue() const {
    if (option_count <= 0 || selected_index < 0 || selected_index >= option_count)
        return "";
    return options[selected_index];
}

void DropDownList::setSelectedValue(const char *value) {
    if (!value) return;
    for (int i = 0; i < option_count; i++) {
        if (strcmp(options[i], value) == 0) {
            setSelectedIndex(i);
            return;
        }
    }
}

void DropDownList::setOpen(bool o) {
    open = o;
    list_need_redraw = true;
    need_redraw = true;
}

bool DropDownList::isOpen() const { return open; }

void DropDownList::setFocused(bool f) {
    focused = f;
    need_redraw = true;
}

void DropDownList::setPosition(int nx, int ny) {
    x = nx;
    y = ny;
    need_redraw = true;
    list_need_redraw = true;
}

void DropDownList::setWidth(int nw) {
    if (nw < 1) nw = 1;
    width = nw;
    need_redraw = true;
    list_need_redraw = true;
}

int DropDownList::measurePreferredWidth() const {
    int max_w = width;
    display->setFont(FORM_FONT);
    display->setTextSize(1);
    for (int i = 0; i < option_count; i++) {
        int16_t tbx, tby;
        uint16_t tbw, tbh;
        display->getTextBounds(options[i], 0, 0, &tbx, &tby, &tbw, &tbh);
        int need = (int) tbw + FORM_TEXT_PAD_X * 2 + 18;
        if (need > max_w) max_w = need;
    }
    const int screen_w = 480;
    if (x + max_w > screen_w)
        max_w = screen_w - x;
    if (max_w < width)
        max_w = width;
    return max_w;
}

void DropDownList::setListMaxHeight(int px) {
    list_max_height = px;
    list_need_redraw = true;
}

bool DropDownList::hitTestField(uint16_t tx, uint16_t ty) const {
    return tx >= (uint16_t) x && tx < (uint16_t) (x + width)
           && ty >= (uint16_t) y && ty < (uint16_t) (y + height);
}

int DropDownList::getListPixelHeight() const {
    int h = option_count * FORM_DROPDOWN_ROW_HEIGHT;
    if (h > list_max_height) h = list_max_height;
    if (h < 0) h = 0;
    return h;
}

bool DropDownList::hitTestList(uint16_t tx, uint16_t ty, int &out_index) const {
    if (!open) return false;
    int list_y = y + height;
    int list_h = getListPixelHeight();
    int list_w = measurePreferredWidth();
    if (tx < (uint16_t) x || tx >= (uint16_t) (x + list_w)
        || ty < (uint16_t) list_y || ty >= (uint16_t) (list_y + list_h))
        return false;
    int row = ((int) ty - list_y + list_scroll) / FORM_DROPDOWN_ROW_HEIGHT;
    if (row < 0 || row >= option_count) return false;
    out_index = row;
    return true;
}

void DropDownList::invalidate() {
    need_redraw = true;
    list_need_redraw = true;
}

void DropDownList::draw() {
    if (!need_redraw) return;
    need_redraw = false;
    const uint16_t bg = focused ? FORM_COLOR_BG_FOCUSED : FORM_COLOR_BG_UNFOCUSED;
    display->fillRect(x, y, width, height, bg);
    display->drawRect(x, y, width, height, FORM_COLOR_BORDER);
    display->setFont(FORM_FONT);
    display->setTextSize(1);
    display->setTextColor(FORM_COLOR_TEXT);
    display->setCursor(x + FORM_TEXT_PAD_X, y + FORM_TEXT_BASELINE);
    display->print(getSelectedValue());
    // chevron
    int cx = x + width - 14;
    int cy = y + height / 2;
    display->fillTriangle(cx - 4, cy - 3, cx + 4, cy - 3, cx, cy + 4, FORM_COLOR_TEXT);
}

void DropDownList::drawListOverlay() {
    if (!open || !list_need_redraw) return;
    list_need_redraw = false;
    int list_y = y + height;
    int list_h = getListPixelHeight();
    int list_w = measurePreferredWidth();
    display->fillRect(x, list_y, list_w, list_h, FORM_COLOR_DROPDOWN_LIST_BG);
    display->drawRect(x, list_y, list_w, list_h, FORM_COLOR_BORDER);

    int first = list_scroll / FORM_DROPDOWN_ROW_HEIGHT;
    int visible = list_h / FORM_DROPDOWN_ROW_HEIGHT;
    for (int i = 0; i < visible; i++) {
        int idx = first + i;
        if (idx >= option_count) break;
        int row_y = list_y + i * FORM_DROPDOWN_ROW_HEIGHT;
        if (idx == selected_index)
            display->fillRect(x + 1, row_y + 1, list_w - 2, FORM_DROPDOWN_ROW_HEIGHT - 2,
                              FORM_COLOR_DROPDOWN_SEL_BG);
        display->setFont(FORM_FONT);
        display->setTextSize(1);
        display->setTextColor(FORM_COLOR_TEXT);
        display->setCursor(x + FORM_TEXT_PAD_X, row_y + FORM_TEXT_BASELINE - 2);
        display->print(options[idx]);
    }
}

void DropDownList::getXY(uint16_t &ox, uint16_t &oy) const { ox = x; oy = y; }
void DropDownList::getWH(uint16_t &w, uint16_t &h) const { w = width; h = height; }
