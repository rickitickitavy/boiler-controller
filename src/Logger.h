/*
 * Logger.h
 *
 *  Created on: 26.06.2017
 *      Author: jane
 */

#ifndef LOGGER_H_
#define LOGGER_H_

#include <Arduino.h>
#include "Defines.h"
#include <freertos/FreeRTOS.h>
#include <freertos/semphr.h>

#define LOGGER_SIZE 100
#define LOG_CAT_NAME "/logs"
#define LOG_FILE_NAME "/logs/console_output.html"
#define MEMORY_LOG_CAPACITY (10 * 1024)

class Logger {
private:
    void print(String msg);

    void println(String msg);

    void initSD();

    void addToFile(const char *msg);

    void flush();

    bool sd_presents;

    void getTime();

    String spanStart = "<span style=\"color: ";
    String spanEnd = "</span>";
    String br = "</br>";

    int collected_lines;
    long last_flushed_time;

    char *datetime_buffer;
    char *mini_datetime_buffer;

    char *memoryLogBuffer;
    size_t memoryLogLength;
    SemaphoreHandle_t memoryLogMutex;


public:
    char logLevel = LOG_LEVEL;

    Logger();

    /**
     * debug
     */
    void add(String msg);

    void copyMemoryLog(String &out);

    void error(String msg);

    void warning(String msg);

    void info(String msg);

    void debug(String msg);

    void detailDebug(String msg);

    void handle();

    bool isSdPresents();
};

extern Logger LOGGER;

#endif /* LOGGER_H_ */
