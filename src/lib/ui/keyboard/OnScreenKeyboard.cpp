#include <Fonts/FreeSans9pt7b.h>
#include <string.h>
#include "OnScreenKeyboard.h"

#define KB_COLOR_BG 0x2945
#define KB_COLOR_KEY 0x4A69
#define KB_COLOR_TEXT 0xFFFF

OnScreenKeyboard::OnScreenKeyboard(ILI9488 *display)
        : display(display), x(0), y(0), width(480), height(128),
          max_len(FORM_MAX_VALUE_LEN - 1), visible(false), shift(false), symbols(false),
          mode(KeyboardMode::ALPHA), need_redraw(false) {
    buffer[0] = 0;
}

void OnScreenKeyboard::setOnChange(ChangeFn change) {
    on_change = change;
}

void OnScreenKeyboard::open(int ox, int oy, int w, int h, const char *initial, int maxlen, KeyboardMode m) {
    x = ox;
    y = oy;
    width = w;
    height = h;
    max_len = maxlen > 0 ? maxlen : (FORM_MAX_VALUE_LEN - 1);
    if (max_len >= FORM_MAX_VALUE_LEN) max_len = FORM_MAX_VALUE_LEN - 1;
    mode = m;
    shift = false;
    symbols = false;
    visible = true;
    buffer[0] = 0;
    if (initial) {
        // Copy only printable prefix so a non-terminated EEPROM string cannot hide
        // trailing garbage that backspace would have to erase first (mqttServer bug).
        int n = 0;
        while (n < max_len) {
            unsigned char c = (unsigned char) initial[n];
            if (c == 0 || c < 32 || c > 126)
                break;
            buffer[n] = (char) c;
            n++;
        }
        buffer[n] = 0;
    }
    need_redraw = true;
}

void OnScreenKeyboard::close() {
    visible = false;
}

bool OnScreenKeyboard::isVisible() const { return visible; }

bool OnScreenKeyboard::contains(uint16_t tx, uint16_t ty) const {
    return visible
           && tx >= (uint16_t) x && tx < (uint16_t) (x + width)
           && ty >= (uint16_t) y && ty < (uint16_t) (y + height);
}

void OnScreenKeyboard::invalidate() { need_redraw = true; }

void OnScreenKeyboard::notifyChange() {
    if (on_change)
        on_change(buffer);
}

void OnScreenKeyboard::appendChar(char c) {
    int len = (int) strlen(buffer);
    if (len >= max_len) return;
    buffer[len] = c;
    buffer[len + 1] = 0;
    notifyChange();
}

void OnScreenKeyboard::backspace() {
    int len = (int) strlen(buffer);
    if (len <= 0) return;
    buffer[len - 1] = 0;
    notifyChange();
}

int OnScreenKeyboard::rowCount() const {
    return 4;
}

// n character keys + a trailing backspace strip (wide enough for reliable touch).
static void rowKeyGeometry(int row_width, int n, int &kw, int &bksp_x, int &bksp_w) {
    if (n < 1) n = 1;
    bksp_w = row_width / 5; // 20% of row
    if (bksp_w < 72)
        bksp_w = 72;
    if (bksp_w > row_width / 3)
        bksp_w = row_width / 3;
    kw = (row_width - bksp_w) / n;
    if (kw < 1) kw = 1;
    bksp_x = n * kw;
    bksp_w = row_width - bksp_x;
}

int OnScreenKeyboard::keyAt(uint16_t tx, uint16_t ty, char &out_char, int &special) const {
    special = SPECIAL_NONE;
    out_char = 0;
    if (!contains(tx, ty))
        return -1;

    const int rows = rowCount();
    const int row_h = height / rows;
    int row = ((int) ty - y) / row_h;
    if (row < 0) row = 0;
    if (row >= rows) row = rows - 1;

    const int local_x = (int) tx - x;

    if (mode == KeyboardMode::NUMERIC) {
        static const char *num_rows[4] = {"123", "456", "789", "-0."};
        const char *keys = num_rows[row];
        int n = (int) strlen(keys);
        int kw, bksp_x, bksp_w;
        rowKeyGeometry(width, n, kw, bksp_x, bksp_w);
        if (local_x >= bksp_x) {
            special = SPECIAL_BKSP;
            return 0;
        }
        int col = local_x / kw;
        if (col < 0) return -1;
        if (col >= n) {
            special = SPECIAL_BKSP;
            return 0;
        }
        out_char = keys[col];
        return 0;
    }

    static const char *alpha_lower[3] = {"qwertyuiop", "asdfghjkl", "zxcvbnm"};
    static const char *alpha_upper[3] = {"QWERTYUIOP", "ASDFGHJKL", "ZXCVBNM"};
    static const char *sym_rows[3] = {"1234567890", "-/:;()$&@", ".,?!'\"#"};
    const char **rows_src = symbols ? sym_rows : (shift ? alpha_upper : alpha_lower);

    // Bottom: mode cycle | Space | backspace ("<") — large reliable delete for IPv4 entry
    if (row == 3) {
        int third = width / 3;
        int col = local_x / third;
        if (col <= 0) { special = SPECIAL_SHIFT; return 0; }
        if (col == 1) { special = SPECIAL_SPACE; return 0; }
        special = SPECIAL_BKSP;
        return 0;
    }

    const char *keys = rows_src[row];
    int n = (int) strlen(keys);
    if (n == 0) return -1;
    int kw, bksp_x, bksp_w;
    rowKeyGeometry(width, n, kw, bksp_x, bksp_w);
    if (local_x >= bksp_x) {
        special = SPECIAL_BKSP;
        return 0;
    }
    int col = local_x / kw;
    if (col < 0) return -1;
    if (col >= n) {
        special = SPECIAL_BKSP;
        return 0;
    }
    out_char = keys[col];
    return 0;
}

bool OnScreenKeyboard::handleTouch(uint16_t tx, uint16_t ty) {
    if (!visible) return false;
    char ch = 0;
    int special = SPECIAL_NONE;
    if (keyAt(tx, ty, ch, special) < 0)
        return false;

    if (special == SPECIAL_BKSP) {
        backspace();
        return true;
    }
    if (special == SPECIAL_SPACE) {
        appendChar(' ');
        return true;
    }
    if (special == SPECIAL_SHIFT) {
        // Cycle: abc → ABC → #+ = → abc (symbols live on this button now)
        if (!symbols && !shift) {
            shift = true;
        } else if (!symbols && shift) {
            shift = false;
            symbols = true;
        } else {
            symbols = false;
            shift = false;
        }
        need_redraw = true;
        return true;
    }
    if (special == SPECIAL_SYM) {
        symbols = !symbols;
        shift = false;
        need_redraw = true;
        return true;
    }
    if (ch) {
        appendChar(ch);
        if (shift && !symbols) {
            shift = false;
            need_redraw = true;
        }
        return true;
    }
    return true;
}

void OnScreenKeyboard::draw() {
    if (!visible || !need_redraw) return;
    need_redraw = false;

    display->fillRect(x, y, width, height, KB_COLOR_BG);
    display->setFont(&FreeSans9pt7b);
    display->setTextSize(1);

    const int rows = rowCount();
    const int row_h = height / rows;

    auto drawKey = [&](int kx, int ky, int kw, int kh, const char *caption, uint16_t bg, uint16_t fg) {
        display->fillRoundRect(kx + 1, ky + 1, kw - 2, kh - 2, 3, bg);
        display->setTextColor(fg);
        int16_t tbx, tby;
        uint16_t tbw, tbh;
        display->getTextBounds(caption, 0, 0, &tbx, &tby, &tbw, &tbh);
        int cx = kx + (kw - (int) tbw) / 2;
        if (cx < kx + 2) cx = kx + 2;
        display->setCursor(cx, ky + (kh + (int) tbh) / 2 - 2);
        display->print(caption);
    };

    if (mode == KeyboardMode::NUMERIC) {
        static const char *num_rows[4] = {"123", "456", "789", "-0."};
        for (int r = 0; r < 4; r++) {
            const char *keys = num_rows[r];
            int n = (int) strlen(keys);
            int kw, bksp_x, bksp_w;
            rowKeyGeometry(width, n, kw, bksp_x, bksp_w);
            for (int c = 0; c < n; c++) {
                char cap[2] = {keys[c], 0};
                drawKey(x + c * kw, y + r * row_h, kw, row_h, cap, KB_COLOR_KEY, KB_COLOR_TEXT);
            }
            drawKey(x + bksp_x, y + r * row_h, bksp_w, row_h, "<", KB_COLOR_KEY, KB_COLOR_TEXT);
        }
        return;
    }

    static const char *alpha_lower[3] = {"qwertyuiop", "asdfghjkl", "zxcvbnm"};
    static const char *alpha_upper[3] = {"QWERTYUIOP", "ASDFGHJKL", "ZXCVBNM"};
    static const char *sym_rows[3] = {"1234567890", "-/:;()$&@", ".,?!'\"#"};
    const char **rows_src = symbols ? sym_rows : (shift ? alpha_upper : alpha_lower);
    for (int r = 0; r < 3; r++) {
        const char *keys = rows_src[r];
        int n = (int) strlen(keys);
        int kw, bksp_x, bksp_w;
        rowKeyGeometry(width, n, kw, bksp_x, bksp_w);
        for (int c = 0; c < n; c++) {
            char cap[2] = {keys[c], 0};
            drawKey(x + c * kw, y + r * row_h, kw, row_h, cap, KB_COLOR_KEY, KB_COLOR_TEXT);
        }
        drawKey(x + bksp_x, y + r * row_h, bksp_w, row_h, "<", KB_COLOR_KEY, KB_COLOR_TEXT);
    }
    int third = width / 3;
    int by = y + 3 * row_h;
    int bh = y + height - by;
    const char *mode_cap = symbols ? "#+=" : (shift ? "ABC" : "abc");
    drawKey(x, by, third, bh, mode_cap, KB_COLOR_KEY, KB_COLOR_TEXT);
    drawKey(x + third, by, third, bh, "Space", KB_COLOR_KEY, KB_COLOR_TEXT);
    drawKey(x + 2 * third, by, third, bh, "<", KB_COLOR_KEY, KB_COLOR_TEXT);
}
