//
// Created by dsporykhin on 12.01.23.
//

#ifndef INC_3_5_INCHES_DSP_ESP32_COSMO_GAUGE_H
#define INC_3_5_INCHES_DSP_ESP32_COSMO_GAUGE_H


#include "lib/adafruit/gfxfont.h"
#include "lib/adafruit/ILI9488.h"
#include <lib/ui/bufferedGraphics/DisplayBuffer.h>
#include <lib/ui/bufferedGraphics/ui_struct.h>

#define GAUGE_GRAPH_MARGIN_BOTTOM 10
#define GAUGE_GRAPH_MARGIN_TOP 2
#define GAUGE_GRAPH_MARGIN_LF_RG 4
#define GAUGE_GRAPH_DEFAULT_WIDTH 15
#define GAUGE_GRAPH_ARROW_ANGLE (PI / 15)

class Gauge {
private:
    ILI9488 *display;
    int width, height;
    const GFXfont *font;
    int display_decimal_digits_count;

    float value;

    float min, max;
    ColorPart *colorParts;

    bool initialized;

    int base_radius;
    int half_arc_len;
    int bg_color, font_color;


public:
    int graph_width = GAUGE_GRAPH_DEFAULT_WIDTH;
    int x, y;
    const char *title;

    DisplayBuffer *defaultDisplayBuffer = nullptr;

    Gauge(ILI9488 *display, const char *title, int x, int y, int width, int height, int bg_color, int font_color, GFXfont *font,
          int display_decimal_digits_count, float min, float max, ColorPart *colorParts);

    float setValue(float value);

    float getValue();

    void draw();

    void setBgColor(int color);
};


#endif //INC_3_5_INCHES_DSP_ESP32_COSMO_GAUGE_H
