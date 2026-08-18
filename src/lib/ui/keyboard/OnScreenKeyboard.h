#ifndef BOILERCONTROLLER_ONSCREENKEYBOARD_H
#define BOILERCONTROLLER_ONSCREENKEYBOARD_H

#include <ILI9488.h>
#include <functional>
#include <lib/ui/form/FormColors.h>

enum class KeyboardMode {
    ALPHA,
    NUMERIC
};

class OnScreenKeyboard {
public:
    typedef std::function<void(const char *text)> ChangeFn;

private:
    ILI9488 *display;
    int x, y, width, height;
    char buffer[FORM_MAX_VALUE_LEN];
    int max_len;
    bool visible;
    bool shift;
    bool symbols;
    KeyboardMode mode;
    ChangeFn on_change;
    bool need_redraw;

    void appendChar(char c);
    void backspace();
    void notifyChange();
    int keyAt(uint16_t tx, uint16_t ty, char &out_char, int &special) const;
    int rowCount() const;

public:
    static const int SPECIAL_NONE = 0;
    static const int SPECIAL_SHIFT = 1;
    static const int SPECIAL_SYM = 2;
    static const int SPECIAL_SPACE = 3;
    static const int SPECIAL_BKSP = 4;

    OnScreenKeyboard(ILI9488 *display);

    void setOnChange(ChangeFn change);
    void open(int x, int y, int width, int height, const char *initial, int max_len, KeyboardMode mode);
    void close();
    bool isVisible() const;
    bool contains(uint16_t tx, uint16_t ty) const;
    bool handleTouch(uint16_t tx, uint16_t ty);
    void invalidate();
    void draw();
    int getHeight() const { return height; }
    int getY() const { return y; }
    const char *getText() const { return buffer; }
};

#endif
