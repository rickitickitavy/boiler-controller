/***************************************************
  STM32 Support added by Jaret Burkett at OSHlab.com

  This is our library for the Adafruit ILI9488 Breakout and Shield
  ----> http://www.adafruit.com/products/1651

  Check out the links above for our tutorials and wiring diagrams
  These displays use SPI to communicate, 4 or 5 pins are required to
  interface (RST is optional)
  Adafruit invests time and resources providing this open source code,
  please support Adafruit and open-source hardware by purchasing
  products from Adafruit!

  Written by Limor Fried/Ladyada for Adafruit Industries.
  MIT license, all text above must be included in any redistribution
 ****************************************************/

#include "ILI9488.h"

#ifdef __AVR
#include <avr/pgmspace.h>
#elif defined(ESP8266)
#include <pgmspace.h>
#endif

#ifndef ARDUINO_STM32_FEATHER

#endif

//#include <asio.hpp>


// If the SPI library has transaction support, these functions
// establish settings and protect from interference from other
// libraries.  Otherwise, they simply do nothing.
#ifdef SPI_HAS_TRANSACTION

static inline void spi_begin(void) __attribute__((always_inline));

static inline void spi_begin(void) {
#if defined (ARDUINO_ARCH_ARC32)
    // max speed!
    SPI.beginTransaction(SPISettings(16000000, MSBFIRST, SPI_MODE0));
#else
    // max speed!
    SPI.beginTransaction(SPISettings(ILI9488_SPI_FREQ, MSBFIRST, SPI_MODE0));
#endif
}

static inline void spi_end(void) __attribute__((always_inline));

static inline void spi_end(void) {
    SPI.endTransaction();
}

#else
#define spi_begin()
#define spi_end()
#endif


// Constructor when using software SPI.  All output pins are configurable.
ILI9488::ILI9488(int8_t cs, int8_t dc, int8_t mosi,
                 int8_t sclk, int8_t rst, int8_t miso) : Adafruit_GFX(ILI9488_TFTWIDTH, ILI9488_TFTHEIGHT) {
    _cs = cs;
    _dc = dc;
    _mosi = mosi;
    _miso = miso;
    _sclk = sclk;
    _rst = rst;
    hwSPI = false;
}


// Constructor when using hardware SPI.  Faster, but must use SPI pins
// specific to each board type (e.g. 11,13 for Uno, 51,52 for Mega, etc.)
ILI9488::ILI9488(int8_t cs, int8_t dc, int8_t rst) : Adafruit_GFX(ILI9488_TFTWIDTH, ILI9488_TFTHEIGHT) {
    _cs = cs;
    _dc = dc;
    _rst = rst;
    hwSPI = true;
    _mosi = _sclk = 0;
}

void ILI9488::spiwrite(uint8_t c) {

    SPI.transfer(c);
}


void ILI9488::writecommand(uint8_t c) {
#if defined(USE_FAST_PINIO) && !defined (_VARIANT_ARDUINO_STM32_)
    *dcport &= ~dcpinmask;
    *csport &= ~cspinmask;
#else
    digitalWrite(_dc, LOW);
    digitalWrite(_sclk, LOW);
    digitalWrite(_cs, LOW);
#endif

    spiwrite(c);

#if defined(USE_FAST_PINIO) && !defined (_VARIANT_ARDUINO_STM32_)
    *csport |= cspinmask;
#else
    digitalWrite(_cs, HIGH);
#endif
}


void ILI9488::writedata(uint8_t c) {
#if defined(USE_FAST_PINIO) && !defined (_VARIANT_ARDUINO_STM32_)
    *dcport |=  dcpinmask;
    *csport &= ~cspinmask;
#else
    digitalWrite(_dc, HIGH);
    digitalWrite(_cs, LOW);
#endif

    spiwrite(c);

#if defined(USE_FAST_PINIO) && !defined (_VARIANT_ARDUINO_STM32_)
    *csport |= cspinmask;
#else
    digitalWrite(_cs, HIGH);
#endif
}


// Rather than a bazillion writecommand() and writedata() calls, screen
// initialization commands and arguments are organized in these tables
// stored in PROGMEM.  The table may look bulky, but that's mostly the
// formatting -- storage-wise this is hundreds of bytes more compact
// than the equivalent code.  Companion function follows.
#define DELAY 0x80


// Companion code to the above tables.  Reads and issues
// a series of LCD commands stored in PROGMEM byte array.
void ILI9488::commandList(uint8_t *addr) {

    uint8_t numCommands, numArgs;
    uint16_t ms;

    numCommands = pgm_read_byte(addr++);   // Number of commands to follow
    while (numCommands--) {                 // For each command...
        writecommand(pgm_read_byte(addr++)); //   Read, issue command
        numArgs = pgm_read_byte(addr++);    //   Number of args to follow
        ms = numArgs & DELAY;          //   If hibit set, delay follows args
        numArgs &= ~DELAY;                   //   Mask out delay bit
        while (numArgs--) {                   //   For each argument...
            writedata(pgm_read_byte(addr++));  //     Read, issue argument
        }

        if (ms) {
            ms = pgm_read_byte(addr++); // Read post-command delay time (ms)
            if (ms == 255) ms = 500;     // If 255, delay for 500 ms
            delay(ms);
        }
    }
}


void ILI9488::begin(void) {
    Serial.println("begin " + String(hwSPI));

    if (_rst > 0) {
        pinMode(_rst, OUTPUT);
        digitalWrite(_rst, LOW);
    }

    pinMode(_dc, OUTPUT);
    pinMode(_cs, OUTPUT);

    Serial.println("hwSPI = " + String(hwSPI));

    if (hwSPI) { // Using hardware SPI
        SPI.begin();

#ifndef SPI_HAS_TRANSACTION
        SPI.setBitOrder(MSBFIRST);
        SPI.setDataMode(SPI_MODE0);
#if defined (_AVR__)
        SPI.setClockDivider(SPI_CLOCK_DIV2); // 8 MHz (full! speed!)
        mySPCR = SPCR;
#elif defined(TEENSYDUINO) || defined (__STM32F1__)
        SPI.setClockDivider(SPI_CLOCK_DIV2); // 8 MHz (full! speed!)
#elif defined (__arm__)
        SPI.setClockDivider(11); // 8-ish MHz (full! speed!)
#endif
#endif
    } else {
        pinMode(_sclk, OUTPUT);
        pinMode(_mosi, OUTPUT);
        pinMode(_miso, INPUT);

#if defined(USE_FAST_PINIO) && !defined (_VARIANT_ARDUINO_STM32_)
        clkport     = portOutputRegister(digitalPinToPort(_sclk));
        clkpinmask  = digitalPinToBitMask(_sclk);
        mosiport    = portOutputRegister(digitalPinToPort(_mosi));
        mosipinmask = digitalPinToBitMask(_mosi);
        *clkport   &= ~clkpinmask;
        *mosiport  &= ~mosipinmask;
#endif
    }

    // toggle RST low to reset
    if (_rst > 0) {
        digitalWrite(_rst, HIGH);
        delay(5);
        digitalWrite(_rst, LOW);
        delay(20);
        digitalWrite(_rst, HIGH);
        delay(150);
    }

    /*
    uint8_t x = readcommand8(ILI9488_RDMODE);
    Serial.print("\nDisplay Power Mode: 0x"); Serial.println(x, HEX);
    x = readcommand8(ILI9488_RDMADCTL);
    Serial.print("\nMADCTL Mode: 0x"); Serial.println(x, HEX);
    x = readcommand8(ILI9488_RDPIXFMT);
    Serial.print("\nPixel Format: 0x"); Serial.println(x, HEX);
    x = readcommand8(ILI9488_RDIMGFMT);
    Serial.print("\nImage Format: 0x"); Serial.println(x, HEX);
    x = readcommand8(ILI9488_RDSELFDIAG);
    Serial.print("\nSelf Diagnostic: 0x"); Serial.println(x, HEX);
  */
    //if(cmdList) commandList(cmdList);

    if (hwSPI) spi_begin();
    // writecommand(0xEF);
    // writedata(0x03);
    // writedata(0x80);
    // writedata(0x02);
    //
    // writecommand(0xCF);
    // writedata(0x00);
    // writedata(0XC1);
    // writedata(0X30);
    //
    // writecommand(0xED);
    // writedata(0x64);
    // writedata(0x03);
    // writedata(0X12);
    // writedata(0X81);
    //
    // writecommand(0xE8);
    // writedata(0x85);
    // writedata(0x00);
    // writedata(0x78);
    //
    // writecommand(0xCB);
    // writedata(0x39);
    // writedata(0x2C);
    // writedata(0x00);
    // writedata(0x34);
    // writedata(0x02);
    //
    // writecommand(0xF7);
    // writedata(0x20);
    //
    // writecommand(0xEA);
    // writedata(0x00);
    // writedata(0x00);
    //
    // writecommand(ILI9488_PWCTR1);    //Power control
    // writedata(0x23);   //VRH[5:0]
    //
    // writecommand(ILI9488_PWCTR2);    //Power control
    // writedata(0x10);   //SAP[2:0];BT[3:0]
    //
    // writecommand(ILI9488_VMCTR1);    //VCM control
    // writedata(0x3e); //¶Ô±È¶Èµ÷½Ú
    // writedata(0x28);
    //
    // writecommand(ILI9488_VMCTR2);    //VCM control2
    // writedata(0x86);  //--
    //
    // writecommand(ILI9488_MADCTL);    // Memory Access Control
    // writedata(0x48);
    //
    // writecommand(ILI9488_PIXFMT);
    // writedata(0x55);
    //
    // writecommand(ILI9488_FRMCTR1);
    // writedata(0x00);
    // writedata(0x18);
    //
    // writecommand(ILI9488_DFUNCTR);    // Display Function Control
    // writedata(0x08);
    // writedata(0x82);
    // writedata(0x27);
    //
    // writecommand(0xF2);    // 3Gamma Function Disable
    // writedata(0x00);
    //
    // writecommand(ILI9488_GAMMASET);    //Gamma curve selected
    // writedata(0x01);
    //
    // writecommand(ILI9488_GMCTRP1);    //Set Gamma
    // writedata(0x0F);
    // writedata(0x31);
    // writedata(0x2B);
    // writedata(0x0C);
    // writedata(0x0E);
    // writedata(0x08);
    // writedata(0x4E);
    // writedata(0xF1);
    // writedata(0x37);
    // writedata(0x07);
    // writedata(0x10);
    // writedata(0x03);
    // writedata(0x0E);
    // writedata(0x09);
    // writedata(0x00);
    //
    // writecommand(ILI9488_GMCTRN1);    //Set Gamma
    // writedata(0x00);
    // writedata(0x0E);
    // writedata(0x14);
    // writedata(0x03);
    // writedata(0x11);
    // writedata(0x07);
    // writedata(0x31);
    // writedata(0xC1);
    // writedata(0x48);
    // writedata(0x08);
    // writedata(0x0F);
    // writedata(0x0C);
    // writedata(0x31);
    // writedata(0x36);
    // writedata(0x0F);


    writecommand(0xE0);
    writedata(0x00);
    writedata(0x03);
    writedata(0x09);
    writedata(0x08);
    writedata(0x16);
    writedata(0x0A);
    writedata(0x3F);
    writedata(0x78);
    writedata(0x4C);
    writedata(0x09);
    writedata(0x0A);
    writedata(0x08);
    writedata(0x16);
    writedata(0x1A);
    writedata(0x0F);


    writecommand(0XE1);
    writedata(0x00);
    writedata(0x16);
    writedata(0x19);
    writedata(0x03);
    writedata(0x0F);
    writedata(0x05);
    writedata(0x32);
    writedata(0x45);
    writedata(0x46);
    writedata(0x04);
    writedata(0x0E);
    writedata(0x0D);
    writedata(0x35);
    writedata(0x37);
    writedata(0x0F);


    writecommand(0XC0);      //Power Control 1
    writedata(0x17);    //Vreg1out
    writedata(0x15);    //Verg2out

    writecommand(0xC1);      //Power Control 2
    writedata(0x41);    //VGH,VGL

    writecommand(0xC5);      //Power Control 3
    writedata(0x00);
    writedata(0x12);    //Vcom
    writedata(0x80);

    writecommand(0x36);      //Memory Access
    writedata(0x48);

    writecommand(0x3A);      // Interface Pixel Format
    writedata(0x66);      //18 bit

    writecommand(0XB0);      // Interface Mode Control
    writedata(0x80);                 //SDO NOT USE

    writecommand(0xB1);      //Frame rate
    writedata(0xA0);    //60Hz

    writecommand(0xB4);      //Display Inversion Control
    writedata(0x02);    //2-dot

    writecommand(0XB6);      //Display Function Control  RGB/MCU Interface Control

    writedata(0x02);    //MCU
    writedata(0x02);    //Source,Gate scan dieection

    writecommand(0XE9);      // Set Image Functio
    writedata(0x00);    // Disable 24 bit data

    writecommand(0xF7);      // Adjust Control
    writedata(0xA9);
    writedata(0x51);
    writedata(0x2C);
    writedata(0x82);    // D7 stream, loose


    writecommand(ILI9488_SLPOUT);    //Exit Sleep
    if (hwSPI) spi_end();
    delay(120);
    if (hwSPI) spi_begin();
    writecommand(ILI9488_DISPON);    //Display on
    if (hwSPI) spi_end();

}

void ILI9488::setScrollArea(uint16_t topFixedArea, uint16_t bottomFixedArea) {
    if (hwSPI) spi_begin();
    writecommand(0x33); // Vertical scroll definition
    writedata(topFixedArea >> 8);
    writedata(topFixedArea);
    writedata((_height - topFixedArea - bottomFixedArea) >> 8);
    writedata(_height - topFixedArea - bottomFixedArea);
    writedata(bottomFixedArea >> 8);
    writedata(bottomFixedArea);
    if (hwSPI) spi_end();
}

void ILI9488::scroll(uint16_t pixels) {
    if (hwSPI) spi_begin();
    writecommand(0x37); // Vertical scrolling start address
    writedata(pixels >> 8);
    writedata(pixels);
    if (hwSPI) spi_end();
}

void ILI9488::setAddrWindow(uint16_t x0, uint16_t y0, uint16_t x1,
                            uint16_t y1) {

    writecommand(ILI9488_CASET); // Column addr set
    writedata(x0 >> 8);
    writedata(x0 & 0xFF);     // XSTART
    writedata(x1 >> 8);
    writedata(x1 & 0xFF);     // XEND

    writecommand(ILI9488_PASET); // Row addr set
    writedata(y0 >> 8);
    writedata(y0 & 0xff);     // YSTART
    writedata(y1 >> 8);
    writedata(y1 & 0xff);     // YEND

    writecommand(ILI9488_RAMWR); // write to RAM

}

void ILI9488::drawImage(const uint8_t *img, uint16_t x, uint16_t y, uint16_t w, uint16_t h) {

    // rudimentary clipping (drawChar w/big text requires this)
    if ((x >= _width) || (y >= _height)) return;
    if ((x + w - 1) >= _width) w = _width - x;
    if ((y + h - 1) >= _height) h = _height - y;

    if (hwSPI) spi_begin();
    setAddrWindow(x, y, x + w - 1, y + h - 1);

    // uint8_t hi = color >> 8, lo = color;

#if defined(USE_FAST_PINIO) && !defined (_VARIANT_ARDUINO_STM32_)
    *dcport |=  dcpinmask;
    *csport &= ~cspinmask;
#else
    digitalWrite(_dc, HIGH);
    digitalWrite(_cs, LOW);
#endif
    uint8_t linebuff[w * 3 + 1];
    uint16_t pixels = w * h;
    // uint16_t count = 0;
    uint32_t count = 0;
    for (uint16_t i = 0; i < h; i++) {
        uint16_t pixcount = 0;
        for (uint16_t o = 0; o < w; o++) {
            uint16_t _temp = img[count + 1] ;
            _temp = (_temp << 8) + img[count];
            color15ToColor24((((uint16_t)img[count + 1]) << 8) + img[count], &linebuff[pixcount]);
            count += 2 ;
            pixcount += 3;
        } // for row
#if defined (__STM32F1__)
        SPI.dmaSend(linebuff, w*3);
#else
        for (uint16_t b = 0; b < w * 3; b++) {
            spiwrite(linebuff[b]);
        }
#endif

    }// for col
#if defined(USE_FAST_PINIO) && !defined (_VARIANT_ARDUINO_STM32_)
    *csport |= cspinmask;
#else
    digitalWrite(_cs, HIGH);
#endif

    if (hwSPI) spi_end();
}


void ILI9488::pushColor(uint16_t color) {
    if (hwSPI) spi_begin();

#if defined(USE_FAST_PINIO) && !defined (_VARIANT_ARDUINO_STM32_)
    *dcport |=  dcpinmask;
    *csport &= ~cspinmask;
#else
    digitalWrite(_dc, HIGH);
    digitalWrite(_cs, LOW);
#endif

    // spiwrite(color >> 8);
    // spiwrite(color);
    // spiwrite(0); // added for 24 bit
    write16BitColor(color);

#if defined(USE_FAST_PINIO) && !defined (_VARIANT_ARDUINO_STM32_)
    *csport |= cspinmask;
#else
    digitalWrite(_cs, HIGH);
#endif

    if (hwSPI) spi_end();
}

void ILI9488::pushColors(uint16_t *data, uint8_t len, boolean first) {
    uint16_t color;
    uint8_t buff[len * 3 + 1];
    uint16_t count = 0;
    uint8_t lencount = len;
    if (hwSPI) spi_begin();
#if defined(USE_FAST_PINIO) && !defined (_VARIANT_ARDUINO_STM32_)
    *csport &= ~cspinmask;
#else
    digitalWrite(_cs, LOW);
#endif
    if (first == true) { // Issue GRAM write command only on first call
#if defined(USE_FAST_PINIO) && !defined (_VARIANT_ARDUINO_STM32_)
        *dcport |=  dcpinmask;
#else
        digitalWrite(_dc, HIGH);
#endif
    }
    while (lencount--) {
        color = *data++;
        buff[count] = ((color & 0xF800) >> 8);
        count++;
        buff[count] = ((color & 0x07E0) >> 3);
        count++;
        buff[count] = ((color & 0x001F) << 3);
        count++;
    }
#if defined (__STM32F1__)
    SPI.dmaSend(buff, len*3);
#else
    for (uint16_t b = 0; b < len * 3; b++) {
        spiwrite(buff[b]);
    }
#endif
#if defined(USE_FAST_PINIO) && !defined (_VARIANT_ARDUINO_STM32_)
    *csport |= cspinmask;
#else
    digitalWrite(_cs, HIGH);
#endif

    if (hwSPI) spi_end();
}

void ILI9488::write16BitColor(uint16_t color) {
    uint8_t color24[3];
    color24[0] = ((color & 0xF800) >> 8); // r
    color24[1] = ((color & 0x07E0) >> 3); // g
    color24[2] = ((color & 0x001F) << 3); // b

    SPI.transfer(color24, 3);
}

void ILI9488::color15ToColor24(uint16_t src, uint8_t *dst) {
    dst[0] = (uint8_t) ((src & 0xF800) >> 8);   // r
    dst[1] = (uint8_t) ((src & 0x07E0) >> 3); // g
    dst[2] = (uint8_t) (src & 0x001F) << 3; // b
}

void ILI9488::writeHorizLine(uint8_t *color24, int len) {
    for (int i = 0; i < len; i++)
        memcpy(&inner_line_buf[i * 3], color24, 3);
    SPI.writeBytes(inner_line_buf, len * 3);
}

void ILI9488::drawPixel(int16_t x, int16_t y, uint16_t color) {

    if ((x < 0) || (x >= _width) || (y < 0) || (y >= _height)) return;

    if (hwSPI) spi_begin();
    setAddrWindow(x, y, x + 1, y + 1);

#if defined(USE_FAST_PINIO) && !defined (_VARIANT_ARDUINO_STM32_)
    *dcport |=  dcpinmask;
    *csport &= ~cspinmask;
#else
    digitalWrite(_dc, HIGH);
    digitalWrite(_cs, LOW);
#endif

    // spiwrite(color >> 8);
    // spiwrite(color);
    // spiwrite(0); // added for 24 bit
    write16BitColor(color);

#if defined(USE_FAST_PINIO) && !defined (_VARIANT_ARDUINO_STM32_)
    *csport |= cspinmask;
#else
    digitalWrite(_cs, HIGH);
#endif

    if (hwSPI) spi_end();
}


void ILI9488::drawFastVLine(int16_t x, int16_t y, int16_t h,
                            uint16_t color) {

    // Rudimentary clipping
    if ((x >= _width) || (y >= _height)) return;

    if ((y + h - 1) >= _height)
        h = _height - y;

    if (hwSPI) spi_begin();
    setAddrWindow(x, y, x, y + h - 1);

//  uint8_t hi = color >> 8, lo = color;

#if defined(USE_FAST_PINIO) && !defined (_VARIANT_ARDUINO_STM32_)
    *dcport |=  dcpinmask;
    *csport &= ~cspinmask;
#else
    digitalWrite(_dc, HIGH);
    digitalWrite(_cs, LOW);
#endif
    uint8_t color24[3];

    color15ToColor24(color, color24);

    writeHorizLine(color24, h);

#if defined(USE_FAST_PINIO) && !defined (_VARIANT_ARDUINO_STM32_)
    *csport |= cspinmask;
#else
    digitalWrite(_cs, HIGH);
#endif

    if (hwSPI) spi_end();
}


void ILI9488::drawFastHLine(int16_t x, int16_t y, int16_t w,
                            uint16_t color) {

    // Rudimentary clipping
    if ((x >= _width) || (y >= _height)) return;
    if ((x + w - 1) >= _width) w = _width - x;
    if (hwSPI) spi_begin();
    setAddrWindow(x, y, x + w - 1, y);

    // uint8_t hi = color >> 8, lo = color;
#if defined(USE_FAST_PINIO) && !defined (_VARIANT_ARDUINO_STM32_)
    *dcport |=  dcpinmask;
    *csport &= ~cspinmask;
#else
    digitalWrite(_dc, HIGH);
    digitalWrite(_cs, LOW);
#endif
    while (w--) {
        // spiwrite(hi);
        // spiwrite(lo);
        // spiwrite(0); // added for 24 bit
        write16BitColor(color);
    }
#if defined(USE_FAST_PINIO) && !defined (_VARIANT_ARDUINO_STM32_)
    *csport |= cspinmask;
#else
    digitalWrite(_cs, HIGH);
#endif
    if (hwSPI) spi_end();
}

void ILI9488::fillScreen(uint16_t color) {
    fillRect(0, 0, _width, _height, color);
}

void ILI9488::fillScreen24Color(int color) {
    fillRect(0, 0, _width, _height, color);
}

void ILI9488::fillRect(int16_t x, int16_t y, int16_t w, int16_t h,
                       uint16_t color) {
    int color24 = (((int) color & 0xF800) << 8) // r
        | (((int) color & 0x07E0) << 5) // g
        | ((int) color & 0x001F) << 3; // b
    fillRect24(x, y, w, h, color24);
}

// fill a rectangle
void ILI9488::fillRect24(int16_t x, int16_t y, int16_t w, int16_t h,
                         int color) {

    // rudimentary clipping (drawChar w/big text requires this)
    if ((x >= _width) || (y >= _height)) return;
    if ((x + w - 1) >= _width) w = _width - x;
    if ((y + h - 1) >= _height) h = _height - y;

    if (hwSPI) spi_begin();
    setAddrWindow(x, y, x + w - 1, y + h - 1);

    // uint8_t hi = color >> 8, lo = color;

    digitalWrite(_dc, HIGH);
    digitalWrite(_cs, LOW);

    uint8_t color24[3];
    color24[0] = (uint8_t) (color >> 16);   // r
    color24[1] = (uint8_t) ((color & 0x00FF00) >> 8); // g
    color24[2] = (uint8_t) (color & 0x00FF); // b

    uint8_t line_buffer[480 * 3];
    for (int i = 0; i < w; i++)
        memcpy(&line_buffer[i * 3], color24, 3);

    for (y = h; y > 0; y--) {
        SPI.writeBytes(line_buffer, w * 3);
    }

    digitalWrite(_cs, HIGH);
    if (hwSPI) spi_end();
}


// Pass 8-bit (each) R,G,B, get back 16-bit packed color
uint16_t ILI9488::color565(uint8_t r, uint8_t g, uint8_t b) {
    return ((r & 0xF8) << 8) | ((g & 0xFC) << 3) | (b >> 3);
}

uint16_t  ILI9488::color24To16(int color24) {
    uint16_t r = (color24 & 0x00f80000) >> 8;
    uint16_t g = (color24 & 0x0000fc00) >> 5;
    uint16_t b = (color24 & 0x000000ff) >> 3;
    return r | g | b ;
}

#define MADCTL_MY  0x80
#define MADCTL_MX  0x40
#define MADCTL_MV  0x20
#define MADCTL_ML  0x10
#define MADCTL_RGB 0x00
#define MADCTL_BGR 0x08
#define MADCTL_MH  0x04

void ILI9488::setRotation(uint8_t m) {

    if (hwSPI) spi_begin();
    writecommand(ILI9488_MADCTL);
    rotation = m % 4; // can't be higher than 3
    switch (rotation) {
        case 0:
            writedata(MADCTL_MX | MADCTL_BGR);
            _width = ILI9488_TFTWIDTH;
            _height = ILI9488_TFTHEIGHT;
            break;
        case 1:
            writedata(MADCTL_MV | MADCTL_BGR);
            _width = ILI9488_TFTHEIGHT;
            _height = ILI9488_TFTWIDTH;
            break;
        case 2:
            writedata(MADCTL_MY | MADCTL_BGR);
            _width = ILI9488_TFTWIDTH;
            _height = ILI9488_TFTHEIGHT;
            break;
        case 3:
            writedata(MADCTL_MX | MADCTL_MY | MADCTL_MV | MADCTL_BGR);
            _width = ILI9488_TFTHEIGHT;
            _height = ILI9488_TFTWIDTH;
            break;
    }
    if (hwSPI) spi_end();
}


void ILI9488::invertDisplay(boolean i) {
    if (hwSPI) spi_begin();
    writecommand(i ? ILI9488_INVON : ILI9488_INVOFF);
    if (hwSPI) spi_end();
}


////////// stuff not actively being used, but kept for posterity


uint8_t ILI9488::spiread(void) {
    uint8_t r = 0;

    if (hwSPI) {
#if defined (__AVR__)
#ifndef SPI_HAS_TRANSACTION
        uint8_t backupSPCR = SPCR;
        SPCR = mySPCR;
#endif
        SPDR = 0x00;
        while(!(SPSR & _BV(SPIF)));
        r = SPDR;

#ifndef SPI_HAS_TRANSACTION
        SPCR = backupSPCR;
#endif
#else
        r = SPI.transfer(0x00);
#endif

    } else {

        for (uint8_t i = 0; i < 8; i++) {
            digitalWrite(_sclk, LOW);
            digitalWrite(_sclk, HIGH);
            r <<= 1;
            if (digitalRead(_miso))
                r |= 0x1;
        }
    }
    //Serial.print("read: 0x"); Serial.print(r, HEX);

    return r;
}

uint8_t ILI9488::readdata(void) {
    digitalWrite(_dc, HIGH);
    digitalWrite(_cs, LOW);
    uint8_t r = spiread();
    digitalWrite(_cs, HIGH);

    return r;
}


uint8_t ILI9488::readcommand8(uint8_t c, uint8_t index) {
    if (hwSPI)
        spi_begin();
    digitalWrite(_dc, LOW); // command
    digitalWrite(_cs, LOW);
    spiwrite(0xD9);  // woo sekret command?
    digitalWrite(_dc, HIGH); // data
    spiwrite(0x10 + index);
    digitalWrite(_cs, HIGH);

    digitalWrite(_dc, LOW);
    digitalWrite(_sclk, LOW);
    digitalWrite(_cs, LOW);
    spiwrite(c);

    digitalWrite(_dc, HIGH);
    uint8_t r = spiread();
    digitalWrite(_cs, HIGH);
    if (hwSPI) spi_end();
    return r;
}



/*

 uint16_t ILI9488::readcommand16(uint8_t c) {
 digitalWrite(_dc, LOW);
 if (_cs)
 digitalWrite(_cs, LOW);

 spiwrite(c);
 pinMode(_sid, INPUT); // input!
 uint16_t r = spiread();
 r <<= 8;
 r |= spiread();
 if (_cs)
 digitalWrite(_cs, HIGH);

 pinMode(_sid, OUTPUT); // back to output
 return r;
 }

 uint32_t ILI9488::readcommand32(uint8_t c) {
 digitalWrite(_dc, LOW);
 if (_cs)
 digitalWrite(_cs, LOW);
 spiwrite(c);
 pinMode(_sid, INPUT); // input!

 dummyclock();
 dummyclock();

 uint32_t r = spiread();
 r <<= 8;
 r |= spiread();
 r <<= 8;
 r |= spiread();
 r <<= 8;
 r |= spiread();
 if (_cs)
 digitalWrite(_cs, HIGH);

 pinMode(_sid, OUTPUT); // back to output
 return r;
 }

 */
