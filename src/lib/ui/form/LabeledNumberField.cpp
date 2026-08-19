#include "LabeledNumberField.h"

LabeledNumberField::LabeledNumberField(ILI9488 *display, int x, int y, int width, const char *label_text,
                                       float min_value, float max_value, bool is_float)
        : label(display, x, y, width),
          field(display, x, y + FORM_LABEL_HEIGHT + FORM_LABEL_FIELD_GAP, width, min_value, max_value, is_float),
          x(x), y(y), width(width) {
    label.setText(label_text);
}

void LabeledNumberField::setLabel(const char *text) { label.setText(text); }
void LabeledNumberField::setNumber(float value) { field.setNumber(value); }
float LabeledNumberField::getNumber() const { return field.getNumber(); }

void LabeledNumberField::setRange(float min_value, float max_value, bool is_float) {
    field.setRange(min_value, max_value, is_float);
}

void LabeledNumberField::setPosition(int nx, int ny) {
    x = nx;
    y = ny;
    label.setPosition(nx, ny);
    field.setPosition(nx, ny + FORM_LABEL_HEIGHT + FORM_LABEL_FIELD_GAP);
}

void LabeledNumberField::setWidth(int nw) {
    if (nw < 1) nw = 1;
    width = nw;
    label.setWidth(nw);
    field.setWidth(nw);
}

void LabeledNumberField::setFocused(bool focused) { field.setFocused(focused); }
bool LabeledNumberField::hitTest(uint16_t tx, uint16_t ty) const { return field.hitTest(tx, ty); }

void LabeledNumberField::invalidate() {
    label.invalidate();
    field.invalidate();
}

void LabeledNumberField::draw() {
    label.draw();
    field.draw();
}

int LabeledNumberField::getTotalHeight() const {
    return FORM_LABEL_HEIGHT + FORM_LABEL_FIELD_GAP + FORM_FIELD_HEIGHT;
}

void LabeledNumberField::getXY(uint16_t &ox, uint16_t &oy) const { ox = x; oy = y; }
