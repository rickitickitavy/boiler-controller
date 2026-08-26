/*
 * Logger.cpp
 *
 *  Created on: 26.06.2017
 *      Author: jane
 */

#include "Logger.h"
#include <stdio.h>
#include <Arduino.h>

#define NEW_LINE_PART_LEN 4

Logger LOGGER;

Logger::Logger() {
#ifdef CON_DEBUG
    Serial.begin(921600);
    Serial.println("Starting logger...");
#endif
    last_flushed_time = 0;
    collected_lines = LOGGER_SIZE;
    datetime_buffer = (char*)malloc(256);
    mini_datetime_buffer = (char*)malloc(256);
    initSD();
}
//------------------------------------------------------------------------------

void Logger::initSD() {
    // SD card logging is not used on this hardware; keep the flag for API compat.
    sd_presents = false;
}
//------------------------------------------------------------------------------

void Logger::flush() {
    if (!sd_presents)
        return;
#ifdef CON_DEBUG
    Serial.println("FLUSHING...");
#endif
    collected_lines = LOGGER_SIZE;
    last_flushed_time = millis();
}
//------------------------------------------------------------------------------

void Logger::addToFile(const char *msg) {
    (void) msg;
}
//------------------------------------------------------------------------------

bool Logger::isSdPresents() {
    return sd_presents;
}
//------------------------------------------------------------------------------

void Logger::add(String msg) {
    (void) msg;
}
//------------------------------------------------------------------------------

void Logger::println(String msg) {
#ifdef CON_DEBUG
    Serial.println(msg);
#else
    (void) msg;
#endif
}
//------------------------------------------------------------------------------

void Logger::print(String msg) {
#ifdef CON_DEBUG
    Serial.print(msg);
#else
    (void) msg;
#endif
}
//------------------------------------------------------------------------------

void Logger::getTime() {
    tm localtm;
    if (getLocalTime(&localtm, 0)) {
        sprintf(mini_datetime_buffer, "%04d-%02d-%02d %02d:%02d:%02d "
                , localtm.tm_year + 1900, localtm.tm_mon, localtm.tm_mday
                , localtm.tm_hour, localtm.tm_min, localtm.tm_sec);
        sprintf(datetime_buffer, "<span style=\"color: gray\">%s</span> ", mini_datetime_buffer);
    } else {
        datetime_buffer[0] = 0;
        mini_datetime_buffer[0] = 0;
    }

}

void Logger::error(String msg) {
    if (logLevel <= LOG_LEVEL_ERROR) {
        getTime();
        println(String(mini_datetime_buffer) + "ERROR: " + msg);
        add(String(datetime_buffer) + spanStart + "red\"><b>ERROR</b>: " + spanEnd + msg + br);
    }
}
//------------------------------------------------------------------------------

void Logger::warning(String msg) {
    if (logLevel <= LOG_LEVEL_WARNING) {
        getTime();
        println(String(mini_datetime_buffer) + "WARNING: " + msg);
        add(String(datetime_buffer) + spanStart + "orange\"><b>WARNING</b>: " + spanEnd + msg + br);
    }
}
//------------------------------------------------------------------------------

void Logger::debug(String msg) {
    if (logLevel <= LOG_LEVEL_DEBUG) {
        getTime();
        println(String(mini_datetime_buffer) + "DEBUG: " + msg);
        add(String(datetime_buffer) + spanStart + "darkGray\"><b>DEBUG</b>: " + spanEnd + msg + br);
    }
}
//------------------------------------------------------------------------------

void Logger::detailDebug(String msg) {
    if (logLevel <= LOG_LEVEL_DETAIL_DEBUG) {
        getTime();
        println(String(mini_datetime_buffer) + "DEBUG: " + msg);
        add(String(datetime_buffer) + spanStart + "darkGray\"><b>DEBUG</b>: " + spanEnd + msg + br);
    }
}
//------------------------------------------------------------------------------

void Logger::info(String msg) {
    if (logLevel <= LOG_LEVEL_INFO) {
        getTime();
        println(String(mini_datetime_buffer) + "INFO: " + msg);
        add(String(datetime_buffer) + spanStart + "black\"><b>INFO</b>: " + spanEnd + msg  + br);
    }
}
//------------------------------------------------------------------------------

void Logger::handle() {
    if ((millis() - last_flushed_time > 30000) && LOGGER_SIZE != collected_lines)
        flush();
}
//------------------------------------------------------------------------------
