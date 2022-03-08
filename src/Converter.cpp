//
// Created by dsporykhin on 08.03.22.
//

#include "Converter.h"

const char Converter::encodeTable[] = "0123456789abcdef";

void Converter::byteToAsciiHex(char *buf, uint8_t src){
    buf[0] = encodeTable[src >> 4];
    buf[1] = encodeTable[src & 0x0F];
}

void Converter::bytesToAsciiHex(char *buf, uint8_t* src, int src_size){
    for (int index = 0; index < src_size; index++){
        byteToAsciiHex(&buf[index << 1], src[index]);
    }
    buf[src_size << 1] = 0;
}

