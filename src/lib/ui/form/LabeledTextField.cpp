#include "LabeledTextField.h"

LabeledTextField::LabeledTextField(ILI9488 *display, int x, int y, int width, const char *label_text)
        : label(display, x, y, width),
          field(display, x, y + FORM_LABEL_HEIGHT + FORM_LABEL_FIELD_GAP, width),
          x(x), y(y), width(width) {
    label.setText(label_text);
}

void LabeledTextField::setLabel(const char *text) { label.setText(text); }
void LabeledTextField::setValue(const char *value) { field.setValue(value); }
const char *LabeledTextField::getValue() const { return field.getValue(); }

void LabeledTextField::setPosition(int nx, int ny) {
    x = nx;
    y = ny;
    label.setPosition(nx, ny);
    field.setPosition(nx, ny + FORM_LABEL_HEIGHT + FORM_LABEL_FIELD_GAP);
}

void LabeledTextField::setWidth(int nw) {
    if (nw < 1) nw = 1;
    width = nw;
    label.setWidth(nw);
    field.setWidth(nw);
}

void LabeledTextField::setFocused(bool focused) { field.setFocused(focused); }
bool LabeledTextField::hitTest(uint16_t tx, uint16_t ty) const { return field.hitTest(tx, ty); }

void LabeledTextField::invalidate() {
    label.invalidate();
    field.invalidate();
}

void LabeledTextField::draw() {
    label.draw();
    field.draw();
}

int LabeledTextField::getTotalHeight() const {
    return FORM_LABEL_HEIGHT + FORM_LABEL_FIELD_GAP + FORM_FIELD_HEIGHT;
}

void LabeledTextField::getXY(uint16_t &ox, uint16_t &oy) const { ox = x; oy = y; }
