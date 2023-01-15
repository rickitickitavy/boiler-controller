//
// Created by dsporykhin on 12.01.23.
//

#include <lib/bufferedGraphics/BufferedGraphic.h>
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

void Gauge::draw() {
    int buf_el_size = width * height;
    uint16_t *buffer = (uint16_t*)malloc(2 * buf_el_size);
    FrameBuffer framebuffer(buffer, width, height);

    for (int i = 0; i < buf_el_size; buffer[i++] = (uint16_t)bg_color);

    ColorPart *_colorParts = colorParts;

    if (!colorParts){
        // no parts. then all gauge will be of blue color
        for (int i = graph_width; i > 0; i--)
            BufferedGraphic::drawArc(framebuffer, (width >> 1), GAUGE_GRAPH_MARGIN_TOP + base_radius, base_radius - i, 0, PI,
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
                BufferedGraphic::drawArc(framebuffer, (width >> 1), GAUGE_GRAPH_MARGIN_TOP + base_radius, base_radius - i, start_angle, theta,
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

        BufferedGraphic::fillTriangle(framebuffer, x1, y1, x_l, y_l, x_r, y_r, font_color);
    }

    display->drawImage((uint8_t*)buffer, x, y, width, height);

    free(buffer);

    // draw arrow
    if (initialized) {
        // draw value
        char  charBuffer[20];
        memset(charBuffer, 0, 20);
        display->setFont(font);

//        display->setCursor(x + (width >> 1))
        display->setTextColor(font_color);
        display->setCursor(x, y + height - GAUGE_GRAPH_MARGIN_BOTTOM);

        String format = "%0." + String(display_decimal_digits_count) + "f";

        Serial.println("draw value " + String(value));
        Serial.println("format= " + format);
        sprintf(charBuffer, format.c_str(), value);
        Serial.println("buf= " + String(charBuffer));
        display->print(charBuffer);
    }

}
