#include <stdio.h>
#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "driver/uart.h"
#include "logger.h"
#include "../include/config.h"

#define LOG_QUEUE_SIZE  32
#define LOG_MSG_MAX     128

typedef struct {
    uint32_t    timestamp_ms;
    log_level_t level;
    char        module[8];
    asrs_err_t  code;
    char        message[LOG_MSG_MAX];
} log_entry_t;

static QueueHandle_t log_queue = NULL;
static const char *level_str[] = {"INFO ", "WARN ", "ERROR"};

static void task_logger(void *pvParams) {
    log_entry_t entry;
    char line[256];
    while (1) {
        if (xQueueReceive(log_queue, &entry, portMAX_DELAY)) {
            snprintf(line, sizeof(line),
                "[%06lu][%s][%-6s][%d] %s\r\n",
                (unsigned long)entry.timestamp_ms,
                level_str[entry.level],
                entry.module,
                entry.code,
                entry.message);
            printf("%s", line);
        }
    }
}

void logger_init(void) {
    log_queue = xQueueCreate(LOG_QUEUE_SIZE, sizeof(log_entry_t));
    xTaskCreate(task_logger, "Task_Logger", STACK_SIZE_LOGGER,
                NULL, PRIORITY_LOGGER, NULL);
}

void log_write(log_level_t level, const char *module, asrs_err_t code, const char *message) {
    if (!log_queue) return;
    log_entry_t entry;
    entry.timestamp_ms = (uint32_t)(xTaskGetTickCount() * portTICK_PERIOD_MS);
    entry.level        = level;
    entry.code         = code;
    strncpy(entry.module,  module,  sizeof(entry.module)  - 1);
    strncpy(entry.message, message, sizeof(entry.message) - 1);
    xQueueSendToBack(log_queue, &entry, 0);
}