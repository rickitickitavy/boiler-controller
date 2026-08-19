#ifndef BOILERCONTROLLER_LABELEDNUMBERFIELD_H
#define BOILERCONTROLLER_LABELEDNUMBERFIELD_H

#include "Label.h"
#include "NumberField.h"

class LabeledNumberField {
private:
    Label label;
    NumberField field;
    int x, y, width;

public:
    LabeledNumberField(ILI9488 *display, int x, int y, int width, const char *label_text,
                       float min_value, float max_value, bool is_float);

    NumberField &getField() { return field; }
    Label &getLabel() { return label; }
    void setLabel(const char *text);
    void setNumber(float value);
    float getNumber() const;
    void setRange(float min_value, float max_value, bool is_float);
    void setPosition(int x, int y);
    void setWidth(int width);
    void setFocused(bool focused);
    bool hitTest(uint16_t tx, uint16_t ty) const;
    void invalidate();
    void draw();
    int getTotalHeight() const;
    void getXY(uint16_t &x, uint16_t &y) const;
};

#endif
