//
// Created by dsporykhin on 12.01.23.
//

#include "lib/adafruit/Fonts/FreeSans12pt7b.h"
#include "Gauge.h"

Gauge::Gauge(ILI9488 *display, int x, int y, int width, int height, int bg_color, int font_color, GFXfont *font,
             int display_decimal_digits_count, float min, float max, ColorPart *colorParts) {

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
    arc_coef = (max - min) / (PI * (float) base_radius);

    Serial.println("----------------------");
    Serial.println("base_radius = " + String(base_radius));
    Serial.println("half_arc_len = " + String(half_arc_len));
    Serial.println("arc_coef = " + String(arc_coef));
}

void Gauge::init() {
    initialized = true;
}

float Gauge::getValue() {
    return value;
}

float Gauge::setValue(float value) {
    if (value < min)
        value = min;
    else if (value > max)
        value = max;

    eraseArrow();
    this->value = value;
    drawArrow();
}

void Gauge::draw_arc(uint16_t *buffer, float cx, float cy, float r, float start_angle, float theta, int points, int color) {
    float px = cx - r * cos(start_angle);
    float py = cy - r * sin(start_angle);

    float dx = px - cx;
    float dy = py - cy;
    float ctheta = cos(theta / (points - 1));
    float stheta = sin(theta / (points - 1));
    for (int i = 1; i != points; ++i) {
        float dxtemp = ctheta * dx - stheta * dy;
        dy = stheta * dx + ctheta * dy;
        dx = dxtemp;
        buffer[(int)(cy + dy) * width + (int)(cx + dx)] = (uint16_t)color;
//        display->drawPixel(cx + dx, cy + dy, color);
    }
}

void Gauge::draw() {
    int buf_el_size = width * height;
    uint16_t *buffer = (uint16_t*)malloc(2 * buf_el_size);

    for (int i = 0; i < buf_el_size; buffer[i++] = (uint16_t)bg_color);

    ColorPart *_colorParts = colorParts;

    if (!colorParts){
        // no parts. then all gauge will be of blue color
        for (int i = graph_width; i > 0; i--)
            draw_arc(buffer, (width >> 1)
                    , GAUGE_GRAPH_MARGIN_TOP + base_radius
                    , base_radius - i
                    , 0 , PI, half_arc_len << 1, ILI9488_BLUE);
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
                draw_arc(buffer, (width >> 1)
                        , GAUGE_GRAPH_MARGIN_TOP + base_radius
                        , base_radius - i
                        , start_angle , theta, half_arc_len << 1, _colorParts->color);

            start_angle += theta;
            started_at = _colorParts->ends_at;

            _colorParts = _colorParts->next;
        }
    }

    display->drawImage((uint8_t*)buffer, x, y, width, height);

    free(buffer);

}
