#ifndef BOILERCONTROLLER_LABEL_H
#define BOILERCONTROLLER_LABEL_H

#include <ILI9488.h>
#include "FormColors.h"

class Label {
private:
    ILI9488 *display;
    int x, y, width, height;
    uint16_t fg_color;
    uint16_t bg_color;
    char text[FORM_MAX_VALUE_LEN];
    bool need_redraw;

public:
    Label(ILI9488 *display, int x, int y, int width, int height = FORM_LABEL_HEIGHT);

    void setText(const char *value);
    const char *getText() const;
    void setPosition(int x, int y);
    void setWidth(int width);
    void setColors(uint16_t fg, uint16_t bg);
    void invalidate();
    void draw();
    void getXY(uint16_t &x, uint16_t &y) const;
    void getWH(uint16_t &w, uint16_t &h) const;
};

#endif
