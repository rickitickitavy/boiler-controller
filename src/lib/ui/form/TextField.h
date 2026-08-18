#ifndef BOILERCONTROLLER_TEXTFIELD_H
#define BOILERCONTROLLER_TEXTFIELD_H

#include <ILI9488.h>
#include "FormColors.h"

class TextField {
private:
    ILI9488 *display;
    int x, y, width, height;
    char value[FORM_MAX_VALUE_LEN];
    bool focused;
    bool need_redraw;

public:
    TextField(ILI9488 *display, int x, int y, int width, int height = FORM_FIELD_HEIGHT);

    void setValue(const char *value);
    const char *getValue() const;
    void setFocused(bool focused);
    bool isFocused() const;
    void setPosition(int x, int y);
    void setWidth(int width);
    bool hitTest(uint16_t tx, uint16_t ty) const;
    void invalidate();
    void draw();
    void getXY(uint16_t &x, uint16_t &y) const;
    void getWH(uint16_t &w, uint16_t &h) const;
};

#endif
