#include "LabeledDropDown.h"

LabeledDropDown::LabeledDropDown(ILI9488 *display, int x, int y, int width, const char *label_text)
        : label(display, x, y, width),
          field(display, x, y + FORM_LABEL_HEIGHT + FORM_LABEL_FIELD_GAP, width),
          x(x), y(y), width(width) {
    label.setText(label_text);
}

void LabeledDropDown::setLabel(const char *text) { label.setText(text); }

void LabeledDropDown::setPosition(int nx, int ny) {
    x = nx;
    y = ny;
    label.setPosition(nx, ny);
    field.setPosition(nx, ny + FORM_LABEL_HEIGHT + FORM_LABEL_FIELD_GAP);
}

void LabeledDropDown::setWidth(int nw) {
    if (nw < 1) nw = 1;
    width = nw;
    label.setWidth(nw);
    field.setWidth(nw);
}

void LabeledDropDown::setFocused(bool focused) { field.setFocused(focused); }
bool LabeledDropDown::hitTest(uint16_t tx, uint16_t ty) const { return field.hitTestField(tx, ty); }

void LabeledDropDown::invalidate() {
    label.invalidate();
    field.invalidate();
}

void LabeledDropDown::draw() {
    label.draw();
    field.draw();
}

int LabeledDropDown::getTotalHeight() const {
    return FORM_LABEL_HEIGHT + FORM_LABEL_FIELD_GAP + FORM_FIELD_HEIGHT;
}

void LabeledDropDown::getXY(uint16_t &ox, uint16_t &oy) const { ox = x; oy = y; }
