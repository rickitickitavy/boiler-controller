//
// Created by dsporykhin on 08.03.21.
//

#include <Logger.h>
#include "DS18x20Hub.h"

DS18x20Hub::DS18x20Hub(int pin) {
    LOGGER.info("Start DS18x20Hub");
    deviceCounter = 0;
    sensor = new OneWire(pin);
    LOGGER.info("Sensor created");
    byte addr[8];

    while (sensor->search(addr)) {
        LOGGER.debug("Found device:");

        char str[17];
        str[16] = 0;
        for (byte i = 0; i < 8; sprintf(&str[2*i], "%02X", addr[i++]));
        LOGGER.debug(str);

        switch (addr[0]) {
            case 0x10:
                LOGGER.debug("  Chip = DS18S20");  // or old DS1820
                break;
            case 0x28:
                LOGGER.debug("  Chip = DS18x20Hub");
                break;
            case 0x22:
                LOGGER.debug("  Chip = DS1822");
                break;
            default:
                LOGGER.error("Device is not a DS18x20Hub family device.");
                return;
        }
        if (OneWire::crc8(addr, 7) != addr[7]) {
            LOGGER.error("CRC is not valid!");
            return;
        } else {
            LOGGER.debug("crc fine");
            // copy to device
            memcpy(&devices[deviceCounter++*8], addr, 8);
        }

    }

}

int DS18x20Hub::getDeviceCount() {
    return deviceCounter;
}

char *DS18x20Hub::devicesToHTMLOptions() {
    buffer[0] = 0;
    for (int deviceIndex = 0; deviceIndex < deviceCounter; deviceIndex++){
        int baseAddr = deviceOptionRepresentationSize * deviceIndex;
        sprintf(&buffer[baseAddr], "<option>");
        for (byte  byteIndex = 0; byteIndex < 8; sprintf(&buffer[baseAddr + 8 + 2 * byteIndex], "%02X", devices[deviceIndex * 8 + byteIndex++]));
        sprintf(&buffer[baseAddr + 8 + 16], "</option>");
    }
    // zerro terminator
    buffer[deviceCounter * deviceOptionRepresentationSize + 1] = 0;
    return buffer;
}

int DS18x20Hub::readSensor(byte *addr) {
//    int16_t raw = (data[1] << 8) | data[0];
//    if (type_s) {
//        raw = raw << 3; // 9 bit resolution default
//        if (data[7] == 0x10) {
//            // "count remain" gives full 12 bit resolution
//            raw = (raw & 0xFFF0) + 12 - data[6];
//        }
//    } else {
//        byte cfg = (data[4] & 0x60);
//        // at lower res, the low bits are undefined, so let's zero them
//        if (cfg == 0x00) raw = raw & ~7;  // 9 bit resolution, 93.75 ms
//        else if (cfg == 0x20) raw = raw & ~3; // 10 bit res, 187.5 ms
//        else if (cfg == 0x40) raw = raw & ~1; // 11 bit res, 375 ms
//        //// default is 12 bit resolution, 750 ms conversion time
//    }
//    Serial.print("  Temperature = ");
//    Serial.println((float)raw / 16.0);

}

