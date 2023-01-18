//
// Created by dsporykhin on 12.01.23.
//

#ifndef INC_3_5_INCHES_DSP_ESP32_COSMO_BUTTON_H
#define INC_3_5_INCHES_DSP_ESP32_COSMO_BUTTON_H


#include <lib/ui/bufferedGraphics/DisplayBuffer.h>
#include "lib/adafruit/gfxfont.h"
#include "lib/adafruit/ILI9488.h"
#include "lib/ui/bufferedGraphics/ui_struct.h"

#define BUTTON_GRAPH_MARGIN_BOTTOM 10
#define BUTTON_GRAPH_MARGIN_TOP 10
#define BUTTON_GRAPH_DEFAULT_WIDTH 15

class Button {
private:
    ILI9488 *display;
    int width, height;
    int bg_color;
    int x, y;
    const char *icon_file_name_on;
    const char *icon_file_name_off;
    int icon_width;
    int icon_height;
    bool state;
    bool need_redraw;

public:
    int graph_width = BUTTON_GRAPH_DEFAULT_WIDTH;
    DisplayBuffer *defaultDisplayBuffer = nullptr;

    Button(ILI9488 *display, int x, int y, int width, int height,
           const char *icon_file_name_on, const char *icon_file_name_off, int icon_width, int icon_height,
           int bg_color);

    float setState(bool state);
//    float getValue();

    void draw();
};


#endif //INC_3_5_INCHES_DSP_ESP32_COSMO_BUTTON_H
