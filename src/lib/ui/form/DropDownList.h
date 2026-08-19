#ifndef BOILERCONTROLLER_DROPDOWNLIST_H
#define BOILERCONTROLLER_DROPDOWNLIST_H

#include <ILI9488.h>
#include "FormColors.h"

class DropDownList {
private:
    ILI9488 *display;
    int x, y, width, height;
    char options[FORM_DROPDOWN_MAX_OPTIONS][FORM_MAX_VALUE_LEN];
    int option_count;
    int selected_index;
    bool open;
    bool focused;
    bool need_redraw;
    bool list_need_redraw;
    int list_scroll;
    int list_max_height;

public:
    DropDownList(ILI9488 *display, int x, int y, int width, int height = FORM_FIELD_HEIGHT);

    void clearOptions();
    bool addOption(const char *value);
    void setSelectedIndex(int index);
    int getSelectedIndex() const;
    const char *getSelectedValue() const;
    void setSelectedValue(const char *value);
    void setOpen(bool open);
    bool isOpen() const;
    void setFocused(bool focused);
    void setPosition(int x, int y);
    void setWidth(int width);
    void setListMaxHeight(int px);
    int measurePreferredWidth() const;
    bool hitTestField(uint16_t tx, uint16_t ty) const;
    bool hitTestList(uint16_t tx, uint16_t ty, int &out_index) const;
    void invalidate();
    void draw();
    void drawListOverlay();
    void getXY(uint16_t &x, uint16_t &y) const;
    void getWH(uint16_t &w, uint16_t &h) const;
    int getListPixelHeight() const;
};

#endif
