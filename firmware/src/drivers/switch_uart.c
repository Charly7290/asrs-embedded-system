#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/semphr.h"
#include "driver/uart.h"
#include "driver/gpio.h"
#include "switch_uart.h"
#include "../include/config.h"
#include "../app/logger.h"

#define SW_FRAME_PREFIX     "SW:"
#define SW_FRAME_PREFIX_LEN 3
#define SW_FRAME_CELLS      12
#define SW_RX_BUF_SIZE      256
#define SW_LINE_MAX         32

static uint16_t          sw_state = 0;
static bool              sw_valid = false;
static SemaphoreHandle_t sw_mutex = NULL;

// --------------------------------------------------------------------------
// Inicializa (o reinicializa) el driver UART1
// --------------------------------------------------------------------------
static void uart_sw_driver_init(void)
{
    // Si ya estaba instalado, desinstalarlo primero
    uart_driver_delete(UART_SW_PORT);

    uart_config_t cfg = {
        .baud_rate  = UART_SW_BAUDRATE,
        .data_bits  = UART_DATA_8_BITS,
        .parity     = UART_PARITY_DISABLE,
        .stop_bits  = UART_STOP_BITS_1,
        .flow_ctrl  = UART_HW_FLOWCTRL_DISABLE,
    };
    uart_param_config(UART_SW_PORT, &cfg);
    uart_set_pin(UART_SW_PORT, UART_SW_TX, UART_SW_RX,
                 UART_PIN_NO_CHANGE, UART_PIN_NO_CHANGE);

    // Pull-up en RX — evita ruido cuando la secundaria no está conectada
    gpio_pullup_en(UART_SW_RX);

    uart_driver_install(UART_SW_PORT, SW_RX_BUF_SIZE, 0, 0, NULL, 0);
}

// --------------------------------------------------------------------------
// Parser de frame "SW:XXXXXXXXXXXX\n"
// --------------------------------------------------------------------------
static bool parse_frame(const char *line, uint16_t *out)
{
    if (strncmp(line, SW_FRAME_PREFIX, SW_FRAME_PREFIX_LEN) != 0) return false;

    const char *data = line + SW_FRAME_PREFIX_LEN;
    for (int i = 0; i < SW_FRAME_CELLS; i++) {
        if (data[i] != '0' && data[i] != '1') return false;
    }
    uint16_t result = 0;
    for (int i = 0; i < SW_FRAME_CELLS; i++) {
        if (data[i] == '1') result |= (1u << i);
    }
    *out = result;
    return true;
}

// --------------------------------------------------------------------------
// Tarea FreeRTOS — lectura con auto-recovery del driver
// --------------------------------------------------------------------------
static void task_switch_uart_rx(void *pvParams)
{
    (void)pvParams;

    uint8_t buf[SW_RX_BUF_SIZE];
    char    line[SW_LINE_MAX];
    int     line_len     = 0;
    int     error_count  = 0;

    while (1) {
        int bytes = uart_read_bytes(UART_SW_PORT, buf, sizeof(buf) - 1,
                                    pdMS_TO_TICKS(100));

        if (bytes < 0) {
            // Error del driver — contar y recuperar si supera el umbral
            error_count++;
            if (error_count >= 10) {
                error_count = 0;
                LOG_W("SW_UART", ERR_COMM_TIMEOUT, "reiniciando driver UART1");
                uart_sw_driver_init();
                line_len = 0;
                vTaskDelay(pdMS_TO_TICKS(200));
            }
            continue;
        }

        // Resetear contador de errores si llegaron bytes válidos
        error_count = 0;

        if (bytes == 0) continue;

        for (int i = 0; i < bytes; i++) {
            char c = (char)buf[i];

            if (c == '\n') {
                line[line_len] = '\0';

                uint16_t parsed = 0;
                if (parse_frame(line, &parsed)) {
                    if (xSemaphoreTake(sw_mutex, pdMS_TO_TICKS(10)) == pdTRUE) {
                        uint16_t prev      = sw_state;
                        bool     was_valid = sw_valid;
                        sw_state = parsed;
                        sw_valid = true;
                        xSemaphoreGive(sw_mutex);

                        if (!was_valid || parsed != prev) {
                            char msg[48];
                            snprintf(msg, sizeof(msg), "switches=0x%03X", parsed);
                            LOG_I("SW_UART", ERR_OK, msg);
                        }
                    }
                } else if (line_len > 0) {
                    LOG_W("SW_UART", ERR_COMM_PARSE, "frame invalido");
                }
                line_len = 0;

            } else if (c != '\r') {
                if (line_len < SW_LINE_MAX - 1) {
                    line[line_len++] = c;
                } else {
                    LOG_W("SW_UART", ERR_COMM_PARSE, "linea demasiado larga");
                    line_len = 0;
                }
            }
        }
    }
}

// --------------------------------------------------------------------------
// API pública
// --------------------------------------------------------------------------
void switch_uart_init(void)
{
    sw_mutex = xSemaphoreCreateMutex();
    uart_sw_driver_init();
    xTaskCreate(task_switch_uart_rx, "Task_SwUartRx", 2048, NULL, 3, NULL);
    LOG_I("SW_UART", ERR_OK, "Receptor switches inicializado");
}

uint16_t switch_uart_get_state(void)
{
    uint16_t state = 0;
    if (sw_mutex && xSemaphoreTake(sw_mutex, pdMS_TO_TICKS(10)) == pdTRUE) {
        state = sw_state;
        xSemaphoreGive(sw_mutex);
    }
    return state;
}

bool switch_uart_cell_occupied(uint8_t col, uint8_t row)
{
    if (col >= 4 || row >= 3) return false;
    uint8_t  index = col * 3 + row;
    uint16_t state = switch_uart_get_state();
    return (state & (1u << index)) != 0;
}

bool switch_uart_has_valid_data(void)
{
    bool valid = false;
    if (sw_mutex && xSemaphoreTake(sw_mutex, pdMS_TO_TICKS(10)) == pdTRUE) {
        valid = sw_valid;
        xSemaphoreGive(sw_mutex);
    }
    return valid;
}