#include "Logger.h"

#define PWM_MAX_WIDTH 1023

#define CON_DEBUG_LEVEL_ALL 1
#define LOG_LEVEL_DETAIL_DEBUG 0
#define LOG_LEVEL_DEBUG 1
#define LOG_LEVEL_INFO 2
#define LOG_LEVEL_WARNING 3
#define LOG_LEVEL_ERROR 4
#define LOG_LEVEL_NOTHING 5

/**
 * Уровень логирования
 */
#define LOG_LEVEL LOG_LEVEL_INFO

#define CON_DEBUG

//#define WIFI_DEFAULT_SID "DIR-842-dde7"
//#define WIFI_DEFAULT_HOST_NAME "lampV2_2"
//#define WIFI_DEFAULT_PASSWORD "16950719"
#define WIFI_DEFAULT_SID "baseWiFi"
#define WIFI_DEFAULT_HOST_NAME "base"
#define WIFI_DEFAULT_PASSWORD "00000000"


#define LOGIN_TOPIC "login"
#define INCOME_COMMAND_TOPIC "cmd_in"
