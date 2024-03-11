//
// Created by dsporykhin on 08.03.22.
//

#include "Converter.h"
#include "Logger.h"

const char Converter::encodeTable[] = "0123456789abcdef";

void Converter::byteToAsciiHex(char *buf, uint8_t src) {
    buf[0] = encodeTable[src >> 4];
    buf[1] = encodeTable[src & 0x0F];
}

void Converter::bytesToAsciiHex(char *buf, uint8_t *src, int src_size) {
    for (int index = 0; index < src_size; index++) {
        byteToAsciiHex(&buf[index << 1], src[index]);
    }
    buf[src_size << 1] = 0;
}

bool Converter::asciiHexDigitToInt4(uint8_t &dst, const char *src) {

    if (*src >= '0' && *src <= '9') {
        // decimal digit
        dst = (uint8_t) (*src - '0');
    } else if (*src >= 'a' && *src <= 'f') {
        dst = (uint8_t) (*src - 'a' + 0x0A);
    } else if (*src >= 'A' && *src <= 'F') {
        dst = (uint8_t) (*src - 'A' + 0x0A);
    } else
        return false;

    return true;
}

bool Converter::asciiHexToBytes(uint8_t *dst, const char *src, int byte_data_len) {
    for (int index = 0; index < byte_data_len; index++) {
        uint8_t temp;

        if ((!asciiHexDigitToInt4(temp, &src[(index << 1) + 1]))
            || (!asciiHexDigitToInt4(dst[index], &src[index << 1])))
            return false;
        dst[index] = (dst[index] << 4) | temp;
    }
    return true;
}

