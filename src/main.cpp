//
// Created by dsporykhin on 13.02.21.
//
#include <Arduino.h>
#include <ArduinoOTA.h>
#include "WiFiController.h"
#include "MqttClient.h"
#include "Switcher.h"

#include <sensor/DS18x20Hub.h>


SettingsManager *settingsManager;
WiFiController *wiFiController;
MqttClient *mqtt;
Switcher *switcher;
DS18x20Hub *sensorsHub;

OneWire *sensor1;
byte data[12];
byte addr[8];
byte type_s;

//bool a0Started;
//long startedAt;
//long lastWorkA;

void setup() {
#ifdef CON_DEBUG
    Serial.begin(74880);
    Serial.println("---");

    LOGGER.info("Started UART at 921600");
#endif

//    a0Started = false;
//    WiFi.begin("TP-LINK_B1C6", "15121820");
//    startedAt = millis();
//    while (!WiFi.isConnected() && (millis() - startedAt < 10000)) {
//        delay(20);
//    }
//
//    if (!WiFi.isConnected()) {
//        LOGGER.info("Not connected: rebooting...");
//        delay(500);
//        ESP.restart();
//    }
//    LOGGER.info("Connected.");
//    LOGGER.info(WiFi.localIP().toString());
//    lastWorkA = millis();
//    pinMode(A0, INPUT);

    sensorsHub = new DS18x20Hub(4);

    LOGGER.info("Starting...");
    LOGGER.info("");
    LOGGER.info(sensorsHub->devicesToHTMLOptions());

//    ArduinoOTA.begin(true);
//
//    settingsManager = new SettingsManager();
//
//    wiFiController = new WiFiController(settingsManager);
//
//    mqtt = new MqttClient(settingsManager->getSettings());
//
//    LOGGER.info("start device");
//
//    switcher = new Switcher(settingsManager->getSettings(), mqtt);
//
//    LOGGER.info("all done");



    Serial.println("scan done");

//    Serial.print("ROM =");
//    for( i = 0; i < 8; i++) {
//        Serial.write(" ");
//        Serial.print(addr[i], HEX);
//    }
//
//    if (OneWire::crc8(addr, 7) != addr[7]) {
//        Serial.println("CRC is not valid!");
//        return;
//    } else {
//        Serial.println("all fine");
//    }
//    Serial.println();
//
//    switch (addr[0]) {
//        case 0x10:
//            Serial.println("  Chip = DS18S20");  // or old DS1820
//            type_s = 1;
//            break;
//        case 0x28:
//            Serial.println("  Chip = DS18x20Hub");
//            type_s = 0;
//            break;
//        case 0x22:
//            Serial.println("  Chip = DS1822");
//            type_s = 0;
//            break;
//        default:
//            Serial.println("Device is not a DS18x20Hub family device.");
//            return;
//    }
//
}


void loop() {
//    sensor1->reset();
//    sensor1->select(addr);
//    sensor1->write(0x44, 1);        // start conversion, with parasite power on at the end
//
//    delay(1000);     // maybe 750ms is enough, maybe not
//    // we might do a ds.depower() here, but the reset will take care of it.
//
//    byte present = sensor1->reset();
//    sensor1->select(addr);
//    sensor1->write(0xBE);         // Read Scratchpad
//
////    Serial.print("  Data = ");
////    Serial.print(present, HEX);
////    Serial.print(" ");
//    for ( byte i = 0; i < 9; i++) {           // we need 9 bytes
//        data[i] = sensor1->read();
////        Serial.print(data[i], HEX);
////        Serial.print(" ");
//    }
////    Serial.print(" CRC=");
////    Serial.print(OneWire::crc8(data, 8), HEX);
////    Serial.println();
//
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



//    ArduinoOTA.handle();
//    mqtt->dispatch();
//    switcher->dispatch();

//    if (millis() > 19000) {
//        if (!a0Started) {
//            a0Started = true;
//            LOGGER.info("A0 started");
//        }
//            if (analogRead(A0) > 0){
//             delay(10);
//            }
//    }


}