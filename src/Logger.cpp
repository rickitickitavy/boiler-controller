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

#define NEW_LINE_PART_LEN 4

Logger LOGGER;

Logger::Logger() {
#ifdef CON_DEBUG
    Serial.begin(921600);
    Serial.println("---");
#endif
}
//------------------------------------------------------------------------------

void Logger::add(String msg) {
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
        add(spanStart + "red\"><b>ERROR</b>: " + msg + spanEnd);
    }
}
//------------------------------------------------------------------------------

void Logger::warning(String msg) {
    if (logLevel <= LOG_LEVEL_WARNING) {
        println("WARNING: " + msg);
        add(spanStart + "orange\"><b>WARNING</b>: " + msg + spanEnd);
    }
}
//------------------------------------------------------------------------------

void Logger::debug(String msg) {
    if (logLevel <= LOG_LEVEL_DEBUG) {
        println("DEBUG: " + msg);
        add(spanStart + "darkGray\"><b>DEBUG</b>: " + msg + spanEnd);
    }
}
//------------------------------------------------------------------------------

void Logger::detailDebug(String msg) {
    if (logLevel <= LOG_LEVEL_DETAIL_DEBUG) {
        println("DEBUG: " + msg);
        add(spanStart + "darkGray\"><b>DEBUG</b>: " + msg + spanEnd);
    }
}
//------------------------------------------------------------------------------

void Logger::info(String msg) {
    if (logLevel <= LOG_LEVEL_INFO) {
        println("INFO: " + msg);
        add(spanStart + "black\"><b>INFO</b>: " + msg + spanEnd);
    }
}
//------------------------------------------------------------------------------

void Logger::saveLogFile() {
}

//------------------------------------------------------------------------------



