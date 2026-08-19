#include <stdlib.h>
#include <stdio.h>
#include "NumberField.h"

NumberField::NumberField(ILI9488 *display, int x, int y, int width, float min_value, float max_value, bool is_float,
                         int height)
        : TextField(display, x, y, width, height), min_value(min_value), max_value(max_value), is_float(is_float) {
}

void NumberField::setRange(float min_v, float max_v, bool as_float) {
    min_value = min_v;
    max_value = max_v;
    is_float = as_float;
}

bool NumberField::commitText(const char *text) {
    if (!text || !text[0])
        return false;
    char *end = nullptr;
    float v = strtof(text, &end);
    if (end == text)
        return false;
    if (v < min_value) v = min_value;
    if (v > max_value) v = max_value;
    setNumber(v);
    return true;
}

void NumberField::setNumber(float value) {
    char buf[32];
    if (is_float)
        snprintf(buf, sizeof(buf), "%.2f", value);
    else
        snprintf(buf, sizeof(buf), "%d", (int) value);
    setValue(buf);
}

float NumberField::getNumber() const {
    return strtof(getValue(), nullptr);
}
