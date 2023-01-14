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

    this->value = value;
}

void Gauge::drawArc(uint16_t *buffer, float cx, float cy, float r, float start_angle, float theta, int points,
                    int color) {
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
    }
}

void Gauge::_swap_int16_t(uint16_t &op1, uint16_t &op2) {
    uint16_t  temp = op1;
    op1 = op2;
    op2 =temp;
}

void Gauge::drawLine(uint16_t *buffer, uint16_t x0, uint16_t y0, uint16_t x1, uint16_t y1, uint16_t color) {
#if defined(ESP8266)
    yield();
#endif
    int16_t steep = abs(y1 - y0) > abs(x1 - x0);
    if (steep) {
        _swap_int16_t(x0, y0);
        _swap_int16_t(x1, y1);
    }

    if (x0 > x1) {
        _swap_int16_t(x0, x1);
        _swap_int16_t(y0, y1);
    }

    int16_t dx, dy;
    dx = x1 - x0;
    dy = abs(y1 - y0);

    int16_t err = dx / 2;
    int16_t ystep;

    if (y0 < y1) {
        ystep = 1;
    } else {
        ystep = -1;
    }

    for (; x0 <= x1; x0++) {
        if (steep) {
            buffer[(int)x0 * width + (int)y0] = color;
        } else {
            buffer[(int)y0 * width + (int)x0] = color;
        }
        err -= dy;
        if (err < 0) {
            y0 += ystep;
            err += dx;
        }
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
            drawArc(buffer, (width >> 1), GAUGE_GRAPH_MARGIN_TOP + base_radius, base_radius - i, 0, PI,
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
                drawArc(buffer, (width >> 1), GAUGE_GRAPH_MARGIN_TOP + base_radius, base_radius - i, start_angle, theta,
                        half_arc_len << 1, _colorParts->color);

            // next arc if exists
            start_angle += theta;
            started_at = _colorParts->ends_at;

            _colorParts = _colorParts->next;
        }
    }

    // draw arrow
    if (initialized){

        float theta = (value - min) * PI / (max - min) - PI / 2;
        float _sin = sin(theta);
        float _cos = cos(theta);
        float _min_radius = base_radius - GAUGE_GRAPH_DEFAULT_WIDTH - 10;
        float _max_radius = base_radius + 2;

        int x0 = _min_radius * _sin + (width >> 1);
        int x1 = _max_radius * _sin + (width >> 1);

        int y0 = base_radius + GAUGE_GRAPH_MARGIN_TOP - _min_radius * _cos;
        int y1 = base_radius + GAUGE_GRAPH_MARGIN_TOP - _max_radius * _cos;

//        drawLine(buffer, x0, y0, x1, y1, font_color);
//        drawLine(buffer, x0 - 1, y0, x1 - 1, y1, font_color);
//        drawLine(buffer, x0 + 1, y0, x1 + 1, y1, font_color);

        float _sin_l = sin(theta - GAUGE_GRAPH_ARROW_ANGLE / 2);
        float _sin_r = sin(theta + GAUGE_GRAPH_ARROW_ANGLE / 2);
        float _cos_l = cos(theta - GAUGE_GRAPH_ARROW_ANGLE / 2);
        float _cos_r = cos(theta + GAUGE_GRAPH_ARROW_ANGLE / 2);

        int x_l = _min_radius * _sin_l + (width >> 1);
        int y_l = base_radius + GAUGE_GRAPH_MARGIN_TOP - _min_radius * _cos_l;
        int x_r = _min_radius * _sin_r + (width >> 1);
        int y_r = base_radius + GAUGE_GRAPH_MARGIN_TOP - _min_radius * _cos_r;

        drawLine(buffer, x1, y1, x_l, y_l, font_color);
        drawLine(buffer, x1, y1, x_r, y_r, font_color);
        drawLine(buffer, x_l, y_l, x_r, y_r, font_color);

    }

    display->drawImage((uint8_t*)buffer, x, y, width, height);

    free(buffer);

}
