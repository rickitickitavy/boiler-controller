//
// Created by dsporykhin on 08.03.22.
//

#ifndef BASE_ESP8266_MQTT_CONVERTER_H
#define BASE_ESP8266_MQTT_CONVERTER_H


#include <cstdint>

class Converter {
private:
    static const char encodeTable[];
public:
    static bool asciiHexDigitToInt4(uint8_t &dst, const char *src);
    static bool asciiHexToBytes(uint8_t *dst, const char *src, int byte_data_len);
    static void byteToAsciiHex(char *buf, uint8_t src);
    static void bytesToAsciiHex(char *buf, uint8_t* src, int src_size);
};


#endif //BASE_ESP8266_MQTT_CONVERTER_H
