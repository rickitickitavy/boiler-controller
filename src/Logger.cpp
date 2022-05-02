/*
 * Logger.cpp
 *
 *  Created on: 26.06.2017
 *      Author: jane
 */

#include "Logger.h"
#include <stdio.h>
#include <Arduino.h>
#include <FS.h>
#include <SD.h>

#define NEW_LINE_PART_LEN 4

Logger LOGGER;

Logger::Logger() {
#ifdef CON_DEBUG
    Serial.begin(115200);
    Serial.println("Starting logger...\r\nTrying to initialize SD card...");
#endif
    last_flushed_time = 0;
    collected_lines = LOGGER_SIZE;
    initSD();
}
//------------------------------------------------------------------------------

void Logger::initSD() {
    sd_presents = false;
    if (!SD.begin(5)) {
#ifdef CON_DEBUG
        Serial.println("SD Card Mount Failed");
#endif
        return;
    } else {
        sd_presents = true;
        uint8_t cardType = SD.cardType();
#ifdef CON_DEBUG
        Serial.printf("SD opened. fs = %i\0", cardType);
#endif
        if (!SD.exists(LOG_CAT_NAME)) {
            SD.mkdir(LOG_CAT_NAME);
        }
        log_file = SD.open(LOG_FILE_NAME, FILE_APPEND);
        const char *_init_message = "</br>--------------  new session started ---------------</br>";
        int wrote = log_file.write((uint8_t *) _init_message, strlen(_init_message));
        if (wrote == 0) {
#ifdef CON_DEBUG
            Serial.println("ERROR!!! - error write to log file");
#endif
            sd_presents = false;
        }
    }
}
//------------------------------------------------------------------------------

void Logger::flush() {
#ifdef CON_DEBUG
    Serial.println("FLUSHING...");
#endif
    collected_lines = LOGGER_SIZE;
    last_flushed_time = millis();
    log_file.flush();
    log_file.close();
    log_file = SD.open(LOG_FILE_NAME, FILE_APPEND);
}
//------------------------------------------------------------------------------



void Logger::addToFile(const char *msg) {
    int wrote = log_file.write((uint8_t *) msg, strlen(msg));
    if (wrote == 0) {
        Serial.println("ERROR!!! - error write to log file");
    } else {
        if (!--collected_lines)
            flush();
    }
}
//------------------------------------------------------------------------------

bool Logger::isSdPresents() {
    return sd_presents;
}
//------------------------------------------------------------------------------

void Logger::add(String msg) {
    if (sd_presents) {
        addToFile(msg.c_str());
    }
//    add((char *) &msg[0]);
}
//------------------------------------------------------------------------------

void Logger::println(String msg) {
#ifdef CON_DEBUG
    Serial.println(msg);
#endif
}
//------------------------------------------------------------------------------

void Logger::print(String msg) {
#ifdef CON_DEBUG
    Serial.print(msg);
#endif
}
//------------------------------------------------------------------------------

void Logger::error(String msg) {
    if (logLevel <= LOG_LEVEL_ERROR) {
        println("ERROR: " + msg);
        add(spanStart + "red\"><b>ERROR</b>: " + msg + spanEnd + br);
    }
}
//------------------------------------------------------------------------------

void Logger::warning(String msg) {
    if (logLevel <= LOG_LEVEL_WARNING) {
        println("WARNING: " + msg);
        add(spanStart + "orange\"><b>WARNING</b>: " + msg + spanEnd + br);
    }
}
//------------------------------------------------------------------------------

void Logger::debug(String msg) {
    if (logLevel <= LOG_LEVEL_DEBUG) {
        println("DEBUG: " + msg);
        add(spanStart + "darkGray\"><b>DEBUG</b>: " + msg + spanEnd + br);
    }
}
//------------------------------------------------------------------------------

void Logger::detailDebug(String msg) {
    if (logLevel <= LOG_LEVEL_DETAIL_DEBUG) {
        println("DEBUG: " + msg);
        add(spanStart + "darkGray\"><b>DEBUG</b>: " + msg + spanEnd + br);
    }
}
//------------------------------------------------------------------------------

void Logger::info(String msg) {
    if (logLevel <= LOG_LEVEL_INFO) {
        println("INFO: " + msg);
        add(spanStart + "black\"><b>INFO</b>: " + msg + spanEnd + br);
    }
}
//------------------------------------------------------------------------------

void Logger::handle() {
    if ((millis() - last_flushed_time > 20000) && LOGGER_SIZE != collected_lines)
        flush();
}
//------------------------------------------------------------------------------



