//
// Created by dsporykhin on 12.01.23.
//

#ifndef INC_3_5_INCHES_DSP_ESP32_COSMO_BUTTON_H
#define INC_3_5_INCHES_DSP_ESP32_COSMO_BUTTON_H


#include <lib/ui/bufferedGraphics/DisplayBuffer.h>
#include <Adafruit_GFX.h>
#include <ILI9488.h>
#include "lib/ui/bufferedGraphics/ui_struct.h"

#define BUTTON_GRAPH_MARGIN_BOTTOM 10
#define BUTTON_GRAPH_MARGIN_TOP 10
#define BUTTON_GRAPH_DEFAULT_WIDTH 15
#define BUTTON_MAX_CAPTION_LENGTH 64

enum DisplayButtonEvent{
    NONE = 0,
    DOWN = 1,
    UP = 2,
    REDRAW = 3
};

typedef std::function<void(DisplayButtonEvent event)> ButtonAction;
typedef std::function<void(DisplayBuffer * canvas)> ButtonDrawAction;


class Button {
private:
    ILI9488 *display;
    int _width, _height;
    int bg_color;
    int x, y;
    const char *icon_file_name_on;
    const char *icon_file_name_off;
    int icon_width;
    int icon_height;
    bool state;
    bool need_redraw;
    char* caption = nullptr;

public:
    int graph_width = BUTTON_GRAPH_DEFAULT_WIDTH;

    DisplayBuffer *defaultDisplayBuffer = nullptr;
    ButtonAction action = nullptr;
    ButtonDrawAction drawAction = nullptr;

    Button(ILI9488 *display, int x, int y, int width, int height,
           const char *icon_file_name_on, const char *icon_file_name_off, int icon_width, int icon_height,
           int bg_color);

    float setState(bool state);
//    float getValue();

    void setCaption(const char* new_caption);
    void draw();
    void getXY(uint16_t &x, uint16_t &y);
    void getWH(uint16_t &w, uint16_t &h);
    void _setWHOnly(uint16_t w, uint16_t h);
    void setBgColor(int color);
};


#endif //INC_3_5_INCHES_DSP_ESP32_COSMO_BUTTON_H
