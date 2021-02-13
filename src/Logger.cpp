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
    SPIFFSConfig cfg;
    cfg.setAutoFormat(false);
    SPIFFS.setConfig(cfg);
    SPIFFS.begin();

    // Пробуем загрузить предыдущий лог
    File oldLogFile = SPIFFS.open("/log", "r");
    if (oldLogFile) {
        logLen = oldLogFile.size();
        oldLogFile.readBytes(&logData[0], oldLogFile.size() > LOGGER_SIZE ? LOGGER_SIZE : oldLogFile.size());
        logData[logLen] = 0;
        oldLogFile.close();
    }
}
//------------------------------------------------------------------------------

void Logger::add(String msg) {
    add((char *) &msg[0]);
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

void Logger::add(char *msg) {
    return ;
    String temp = String(msg);
    temp.replace("\r\n", "<br>");

    msg = &temp[0];
    int32_t msgLen = strlen(msg);
    if ((msgLen + NEW_LINE_PART_LEN + logLen) > LOGGER_SIZE) {
        int8_t startCopyPos = msgLen + NEW_LINE_PART_LEN + logLen - LOGGER_SIZE;
        memmove(logData, logData + startCopyPos, LOGGER_SIZE - startCopyPos);
        logLen = LOGGER_SIZE - msgLen - NEW_LINE_PART_LEN;
    }
    int32_t posToInsert = logLen;
    logLen = logLen + msgLen + NEW_LINE_PART_LEN;
    memmove(logData + posToInsert, msg, msgLen);
    logData[logLen - 4] = '<';
    logData[logLen - 3] = 'b';
    logData[logLen - 2] = 'r';
    logData[logLen - 1] = '>';
}
//------------------------------------------------------------------------------

void Logger::saveLogFile() {
    // Пробуем сохранить лог
    println("Tryin to save log");
    File logFile = SPIFFS.open("/log", "w");
    if (logFile) {
        println("saving log");
        logFile.truncate(0);
        logFile.write(&logData[0], logLen);
        logFile.flush();
        logFile.close();
        println("log saved to /log");
    }

}

//------------------------------------------------------------------------------



