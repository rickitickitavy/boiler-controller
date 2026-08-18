#ifndef BOILERCONTROLLER_NUMBERFIELD_H
#define BOILERCONTROLLER_NUMBERFIELD_H

#include "TextField.h"

class NumberField : public TextField {
private:
    float min_value;
    float max_value;
    bool is_float;

public:
    NumberField(ILI9488 *display, int x, int y, int width, float min_value, float max_value, bool is_float,
                int height = FORM_FIELD_HEIGHT);

    float getMin() const { return min_value; }
    float getMax() const { return max_value; }
    bool isFloat() const { return is_float; }
    void setRange(float min_value, float max_value, bool is_float);
    bool commitText(const char *text);
    void setNumber(float value);
    float getNumber() const;
};

#endif
