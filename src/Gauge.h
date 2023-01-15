//
// Created by dsporykhin on 12.01.23.
//

#ifndef INC_3_5_INCHES_DSP_ESP32_COSMO_GAUGE_H
#define INC_3_5_INCHES_DSP_ESP32_COSMO_GAUGE_H


#include "lib/adafruit/gfxfont.h"
#include "lib/adafruit/ILI9488.h"
#include "lib/bufferedGraphics/DisplayBuffer.h"

#define GAUGE_GRAPH_MARGIN_BOTTOM 10
#define GAUGE_GRAPH_MARGIN_TOP 2
#define GAUGE_GRAPH_MARGIN_LF_RG 4
#define GAUGE_GRAPH_DEFAULT_WIDTH 15
#define GAUGE_GRAPH_ARROW_ANGLE (PI / 15)

struct ColorPart {
    uint16_t color;
    float ends_at;
    ColorPart *next; // if next is NULL then this part extends to the end
    ColorPart(int color,
              float ends_at,
              ColorPart *next) {
        this->next = next;
        this->ends_at = ends_at;
        this->color = DisplayBuffer::color24To16(color);
    }
};

class Gauge {
private:
    ILI9488 *display;
    int width, height;
    int bg_color, font_color;
    const GFXfont *font;
    int display_decimal_digits_count;

    float value;

    float min, max;
    ColorPart *colorParts;

    bool initialized;

    int base_radius;
    int half_arc_len;
    float arc_coef;


public:
    int graph_width = GAUGE_GRAPH_DEFAULT_WIDTH;
    int x, y;
    const char *title;

    Gauge(ILI9488 *display, const char *title, int x, int y, int width, int height, int bg_color, int font_color, GFXfont *font,
          int display_decimal_digits_count, float min, float max, ColorPart *colorParts);

    void init();

    float setValue(float value);

    float getValue();

    void draw();
};


#endif //INC_3_5_INCHES_DSP_ESP32_COSMO_GAUGE_H
