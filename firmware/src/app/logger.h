#pragma once

#include "freertos/FreeRTOS.h"
#include "freertos/queue.h"
#include "../include/error_codes.h"

typedef enum {
    LOG_INFO  = 0,
    LOG_WARN  = 1,
    LOG_ERROR = 2,
} log_level_t;

void logger_init(void);
void log_write(log_level_t level, const char *module, asrs_err_t code, const char *message);

#define LOG_I(mod, code, msg)  log_write(LOG_INFO,  mod, code, msg)
#define LOG_W(mod, code, msg)  log_write(LOG_WARN,  mod, code, msg)
#define LOG_E(mod, code, msg)  log_write(LOG_ERROR, mod, code, msg)