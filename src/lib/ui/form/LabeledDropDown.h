#ifndef BOILERCONTROLLER_LABELEDDROPDOWN_H
#define BOILERCONTROLLER_LABELEDDROPDOWN_H

#include "Label.h"
#include "DropDownList.h"

class LabeledDropDown {
private:
    Label label;
    DropDownList field;
    int x, y, width;

public:
    LabeledDropDown(ILI9488 *display, int x, int y, int width, const char *label_text);

    DropDownList &getField() { return field; }
    Label &getLabel() { return label; }
    void setLabel(const char *text);
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
