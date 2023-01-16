//
// Created by dsporykhin on 12.01.23.
//

#include <lib/bufferedGraphics/DisplayBuffer.h>
#include <lib/adafruit/gfxfont.h>
#include "lib/adafruit/Fonts/FreeSans12pt7b.h"
#include "Gauge.h"

Gauge::Gauge(ILI9488 *display, const char *title, int x, int y, int width, int height, int bg_color, int font_color, GFXfont *font,
             int display_decimal_digits_count, float min, float max, ColorPart *colorParts) {


    this->title = title;
    this->x = x;
    this->y = y;
    this->width = width;
    this->height = height;

    this->bg_color = bg_color;
    this->font_color = font_color;
    this->font = font ? font : &FreeSans12pt7b;
    this->display_decimal_digits_count = display_decimal_digits_count;

    this->min = min;
    this->max = max;
    this->colorParts = colorParts;

    this->value = 0;
    this->display = display;

    // calculate radius of gauge
    int radius_h = height - GAUGE_GRAPH_MARGIN_BOTTOM - GAUGE_GRAPH_MARGIN_TOP;
    int radius_w = (width >> 1) - GAUGE_GRAPH_MARGIN_LF_RG;
    base_radius = radius_h > radius_w ? radius_w : radius_h;

    half_arc_len = (int)(PI * base_radius);
    initialized = false;
}

float Gauge::getValue() {
    return value;
}

float Gauge::setValue(float value) {
    this->value = value;
    initialized = true;
}

void Gauge::draw() {
    DisplayBuffer *displayBuffer = defaultDisplayBuffer ? defaultDisplayBuffer : new DisplayBuffer(width, height);
    displayBuffer->fillScreen(bg_color);

    ColorPart *_colorParts = colorParts;

    if (!colorParts){
        // no parts. then all gauge will be of blue color
        for (int i = graph_width; i > 0; i--)
            displayBuffer->drawArc((width >> 1), GAUGE_GRAPH_MARGIN_TOP + base_radius, base_radius - i, 0, PI,
                    half_arc_len << 1, ILI9488_BLUE);
    } else {
        float start_angle = 0;
        float full_diapason = max - min;
        float started_at = min;
        while (_colorParts){
            // calc angle
            float ends_at = _colorParts->ends_at;
            if (!_colorParts->next)
                ends_at = max;
            float theta = (ends_at - started_at) * PI / full_diapason;

            // draw
            for (int i = graph_width; i > 0; i--)
                displayBuffer->drawArc((width >> 1), GAUGE_GRAPH_MARGIN_TOP + base_radius, base_radius - i, start_angle, theta,
                                       (int)(half_arc_len * theta / PI) << 1, _colorParts->color);

            // next arc if exists
            start_angle += theta;
            started_at = _colorParts->ends_at;

            _colorParts = _colorParts->next;
        }
    }

    displayBuffer->setFont(font);
    displayBuffer->setTextColor(font_color);

    // draw arrow
    if (initialized){
        float temp_value = value;
        if (temp_value < min)
            temp_value = min;
        else if (temp_value > max)
            temp_value = max;


        float theta = (temp_value - min) * PI / (max - min) - PI / 2;
        float _sin = sin(theta);
        float _cos = cos(theta);
        float _min_radius = base_radius - GAUGE_GRAPH_DEFAULT_WIDTH - 10;
        float _max_radius = base_radius + 2;

        int x1 = _max_radius * _sin + (width >> 1);

        int y1 = base_radius + GAUGE_GRAPH_MARGIN_TOP - _max_radius * _cos;

        float _sin_l = sin(theta - GAUGE_GRAPH_ARROW_ANGLE / 2);
        float _sin_r = sin(theta + GAUGE_GRAPH_ARROW_ANGLE / 2);
        float _cos_l = cos(theta - GAUGE_GRAPH_ARROW_ANGLE / 2);
        float _cos_r = cos(theta + GAUGE_GRAPH_ARROW_ANGLE / 2);

        int x_l = _min_radius * _sin_l + (width >> 1);
        int y_l = base_radius + GAUGE_GRAPH_MARGIN_TOP - _min_radius * _cos_l;
        int x_r = _min_radius * _sin_r + (width >> 1);
        int y_r = base_radius + GAUGE_GRAPH_MARGIN_TOP - _min_radius * _cos_r;

        displayBuffer->fillTriangle(x1, y1, x_l, y_l, x_r, y_r, font_color);

        // draw value
        char  charBuffer[20];
        memset(charBuffer, 0, 20);
        String format = "%0." + String(display_decimal_digits_count) + "f";
        sprintf(charBuffer, format.c_str(), value);

        displayBuffer->setCursor((width - displayBuffer->calcTextWidth(charBuffer)) >> 1,
                                 GAUGE_GRAPH_MARGIN_TOP + base_radius + (font->yAdvance >> 1));

        displayBuffer->print(charBuffer);

    }

    //draw caption
    int x_t = (int)width - (int)displayBuffer->calcTextWidth(title);
    if (x_t < 0)
        x_t = 0;

    x_t = 2;
    displayBuffer->setCursor(x_t >> 1, height - 3);
    displayBuffer->print(title);

    display->drawImage((uint8_t*)displayBuffer->buffer, x, y, width, height);
    if (!defaultDisplayBuffer) {
        displayBuffer->freeBuffer();
        free(displayBuffer);
    }

}
