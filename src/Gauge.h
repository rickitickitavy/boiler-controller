//
// Created by dsporykhin on 12.01.23.
//

#ifndef INC_3_5_INCHES_DSP_ESP32_COSMO_GAUGE_H
#define INC_3_5_INCHES_DSP_ESP32_COSMO_GAUGE_H


#include "lib/adafruit/gfxfont.h"
#include "lib/adafruit/ILI9488.h"

#define GAUGE_GRAPH_MARGIN_BOTTOM 10
#define GAUGE_GRAPH_MARGIN_TOP 10
#define GAUGE_GRAPH_MARGIN_LF_RG 4
#define GAUGE_GRAPH_DEFAULT_WIDTH 15

struct ColorPart {
    int color;
    float ends_at;
    ColorPart *next; // if next is NULL then this part extends to the end
    ColorPart(int color,
              float ends_at,
              ColorPart *next) {
        this->next = next;
        this->ends_at = ends_at;
        this->color = color;
    }
};

class Gauge {
private:
    ILI9488 *display;
    int x, y, width, height;
    int bg_color, font_color;
    const GFXfont *font;
    int display_digits_count;

    float value;

    float min, max;
    ColorPart *colorParts;

    bool initialized;

    int base_radius;
    int center_y;
    int arrow_position;
    int half_arc_len;
    float arc_coef;

    void eraseArrow();

    void drawArrow();

    void draw_part(int color, int start_value, float end_value);

    void draw_arc(uint16_t *buffer, float cx, float cy, float r, float start_angle, float theta, int points, int color);

public:
    int graph_width = GAUGE_GRAPH_DEFAULT_WIDTH;

    Gauge(ILI9488 *display, int x, int y, int width, int height, int bg_color, int font_color, GFXfont *font,
          int display_digits_count, float min, float max, ColorPart *colorParts);

    void init();

    float setValue(float value);

    float getValue();

    void draw(bool draw_background);
};


#endif //INC_3_5_INCHES_DSP_ESP32_COSMO_GAUGE_H
