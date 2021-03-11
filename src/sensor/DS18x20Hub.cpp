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

}

