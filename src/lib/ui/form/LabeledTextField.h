#ifndef BOILERCONTROLLER_LABELEDTEXTFIELD_H
#define BOILERCONTROLLER_LABELEDTEXTFIELD_H

#include "Label.h"
#include "TextField.h"

class LabeledTextField {
private:
    Label label;
    TextField field;
    int x, y, width;

public:
    LabeledTextField(ILI9488 *display, int x, int y, int width, const char *label_text);

    TextField &getField() { return field; }
    Label &getLabel() { return label; }
    void setLabel(const char *text);
    void setValue(const char *value);
    const char *getValue() const;
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
