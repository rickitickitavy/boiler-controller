#include "PwmPCA9685Driver.h"
#include <Arduino.h>
#include <Wire.h>
#include "Logger.h"

#define MODE1_REG (uint8_t)0b00100000
#define MODE2_REG (uint8_t)0b00000100

PwmPCA9685Driver::PwmPCA9685Driver(char addr) {

    String scaninfo = "Scan I2C devices:\r\n";
    for (int _addr = 0; _addr < 128; _addr++){
        Wire.beginTransmission(_addr);
        if(Wire.endTransmission() == 0){
            scaninfo += String(_addr) + "\r\n";
        }
    }
    LOGGER.info(scaninfo);

	detected = false;
	this->addr = addr;


	Wire.beginTransmission(addr);
	Wire.write((uint8_t)0x00);
	Wire.write(MODE1_REG);
	Wire.write(MODE2_REG);
	int rslt = Wire.endTransmission();

	if (rslt == 0) {
		for (char i = 0; i < 16; i ++){
			setChannelValue(i, 0);
			delay(0);
		}
		LOGGER.info("all PWM stopped");
		detected = true;
	} else {
		LOGGER.error("init PWM driver error");
	}
}
//----------------------------------------------------

unsigned char PwmPCA9685Driver::setChannelValue(unsigned char channel,
		int value) {
	unsigned char offHByte, offLByte;

	if (value == 0) {
		offLByte = 0;
		offHByte = 0x10;
	} else {
		offLByte = value & 0xFF;
		offHByte = (value >> 8) & 0x0F;
	}

	Wire.beginTransmission(addr);
	Wire.write((uint8_t)(0x06 + (channel << 2)));
	Wire.write((uint8_t)0);
	Wire.write((uint8_t)0);
	Wire.write(offLByte);
	Wire.write(offHByte);
	return Wire.endTransmission();
}
//----------------------------------------------------

unsigned char PwmPCA9685Driver::setPwmFrequency(int frequency) {
	int rslt;
	Wire.beginTransmission(addr);
	Wire.write((uint8_t)0x00);
	Wire.write((uint8_t)(MODE1_REG | 0b00010000));
	rslt = Wire.endTransmission();
	if (rslt == 0) {
		delay(3);

		unsigned char freqParam = (unsigned char) (25000000 / (4096 * frequency));

		LOGGER.info("set PWM frequensy to "	+ String(25000000 / (long)freqParam / (long)4096) + "Hz (" + String(freqParam) + ")");
		Wire.beginTransmission(addr);
		Wire.write((uint8_t)0xFE);
		Wire.write(freqParam);
		rslt = Wire.endTransmission();
		if (rslt == 0) {
			delay(3);
			Wire.beginTransmission(addr);
			Wire.write((uint8_t)0x00);
			Wire.write(MODE1_REG);
			rslt = Wire.endTransmission();
		}
	}

	if (rslt != 0) {
		LOGGER.error(" set PWM frequency failed");
	}

	return rslt;
}


