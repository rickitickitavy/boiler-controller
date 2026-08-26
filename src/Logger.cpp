/*
 * Logger.cpp
 *
 *  Created on: 26.06.2017
 *      Author: jane
 */

#include "Logger.h"
#include <stdio.h>
#include <string.h>
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
    memoryLogLength = 0;
    memoryLogBuffer = (char *) malloc(MEMORY_LOG_CAPACITY + 1);
    if (memoryLogBuffer != nullptr) {
        memoryLogBuffer[0] = 0;
    }
    memoryLogMutex = xSemaphoreCreateMutex();
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
    if (memoryLogBuffer == nullptr)
        return;

    const char *incomingText = msg.c_str();
    size_t incomingLength = msg.length();
    size_t needed = incomingLength + 1;

    if (memoryLogMutex != nullptr)
        xSemaphoreTake(memoryLogMutex, portMAX_DELAY);

    if (needed > MEMORY_LOG_CAPACITY) {
        size_t keepLength = MEMORY_LOG_CAPACITY - 1;
        memcpy(memoryLogBuffer, incomingText + (incomingLength - keepLength), keepLength);
        memoryLogBuffer[keepLength] = '\n';
        memoryLogLength = MEMORY_LOG_CAPACITY;
    } else {
        while (memoryLogLength + needed > MEMORY_LOG_CAPACITY && memoryLogLength > 0) {
            char *firstNewline = (char *) memchr(memoryLogBuffer, '\n', memoryLogLength);
            if (firstNewline == nullptr) {
                memoryLogLength = 0;
                break;
            }
            size_t dropCount = (size_t) (firstNewline - memoryLogBuffer) + 1;
            memoryLogLength -= dropCount;
            memmove(memoryLogBuffer, firstNewline + 1, memoryLogLength);
        }
        memcpy(memoryLogBuffer + memoryLogLength, incomingText, incomingLength);
        memoryLogLength += incomingLength;
        memoryLogBuffer[memoryLogLength] = '\n';
        memoryLogLength++;
    }
    memoryLogBuffer[memoryLogLength] = 0;

    if (memoryLogMutex != nullptr)
        xSemaphoreGive(memoryLogMutex);
}
//------------------------------------------------------------------------------

void Logger::copyMemoryLog(String &out) {
    if (memoryLogBuffer == nullptr) {
        out = "";
        return;
    }
    if (memoryLogMutex != nullptr)
        xSemaphoreTake(memoryLogMutex, portMAX_DELAY);
    out = String(memoryLogBuffer, memoryLogLength);
    if (memoryLogMutex != nullptr)
        xSemaphoreGive(memoryLogMutex);
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
        String line = String(mini_datetime_buffer) + "ERROR: " + msg;
        println(line);
        add(line);
    }
}
//------------------------------------------------------------------------------

void Logger::warning(String msg) {
    if (logLevel <= LOG_LEVEL_WARNING) {
        getTime();
        String line = String(mini_datetime_buffer) + "WARNING: " + msg;
        println(line);
        add(line);
    }
}
//------------------------------------------------------------------------------

void Logger::debug(String msg) {
    if (logLevel <= LOG_LEVEL_DEBUG) {
        getTime();
        String line = String(mini_datetime_buffer) + "DEBUG: " + msg;
        println(line);
        add(line);
    }
}
//------------------------------------------------------------------------------

void Logger::detailDebug(String msg) {
    if (logLevel <= LOG_LEVEL_DETAIL_DEBUG) {
        getTime();
        String line = String(mini_datetime_buffer) + "DEBUG: " + msg;
        println(line);
        add(line);
    }
}
//------------------------------------------------------------------------------

void Logger::info(String msg) {
    if (logLevel <= LOG_LEVEL_INFO) {
        getTime();
        String line = String(mini_datetime_buffer) + "INFO: " + msg;
        println(line);
        add(line);
    }
}
//------------------------------------------------------------------------------

void Logger::handle() {
    if ((millis() - last_flushed_time > 30000) && LOGGER_SIZE != collected_lines)
        flush();
}
//------------------------------------------------------------------------------
