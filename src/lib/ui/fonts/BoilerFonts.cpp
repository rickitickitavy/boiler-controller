#include "BoilerFonts.h"

#include <Fonts/FreeSans12pt7b.h>
#include <Fonts/FreeSans9pt7b.h>

// Adafruit font headers use namespace-scope const (internal linkage), so each
// #include would duplicate flash. Include them once here and export pointers.
const GFXfont *const FONT_FREE_SANS_9PT = &FreeSans9pt7b;
const GFXfont *const FONT_FREE_SANS_12PT = &FreeSans12pt7b;
