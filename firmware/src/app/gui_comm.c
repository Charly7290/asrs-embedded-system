#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "driver/uart.h"
#include "gui_comm.h"
#include "../include/config.h"
#include "../app/logger.h"

// --------------------------------------------------------------------------
// Constantes
// --------------------------------------------------------------------------
#define GUI_RX_BUF    256
#define GUI_LINE_MAX  64
#define GUI_CMD_QUEUE 8     // máx comandos pendientes sin procesar

// --------------------------------------------------------------------------
// Cola interna de comandos parseados
// --------------------------------------------------------------------------
static QueueHandle_t cmd_queue = NULL;

// --------------------------------------------------------------------------
// Parser de celda — convierte "A1" → col=0, row=0
// Retorna true si el formato es válido.
// --------------------------------------------------------------------------
static bool parse_cell(const char *token, uint8_t *col, uint8_t *row)
{
    if (!token || token[0] == '\0' || token[1] == '\0') return false;

    char col_char = token[0];
    int  row_num  = atoi(&token[1]);

    if (col_char < 'A' || col_char > 'D') return false;
    if (row_num  < 1   || row_num  > 3  ) return false;

    *col = (uint8_t)(col_char - 'A');   // A=0, B=1, C=2, D=3
    *row = (uint8_t)(row_num  - 1);     // 1=0, 2=1, 3=2
    return true;
}

// --------------------------------------------------------------------------
// Parser de línea completa → asrs_cmd_t
// --------------------------------------------------------------------------
static bool parse_line(const char *line, asrs_cmd_t *cmd)
{
    memset(cmd, 0, sizeof(*cmd));

    // RESET — sin argumentos
    if (strcmp(line, "RESET") == 0) {
        cmd->type = CMD_RESET;
        return true;
    }

    // STORE:XN
    if (strncmp(line, "STORE:", 6) == 0) {
        cmd->type = CMD_STORE;
        return parse_cell(line + 6, &cmd->col1, &cmd->row1);
    }

    // RETRIEVE:XN
    if (strncmp(line, "RETRIEVE:", 9) == 0) {
        cmd->type = CMD_RETRIEVE;
        return parse_cell(line + 9, &cmd->col1, &cmd->row1);
    }

    // SWAP:XN:XN
    if (strncmp(line, "SWAP:", 5) == 0) {
        cmd->type = CMD_SWAP;
        // Buscar el segundo ':'
        const char *p = line + 5;
        char cell1[4] = {0};
        const char *sep = strchr(p, ':');
        if (!sep) return false;
        size_t len1 = sep - p;
        if (len1 == 0 || len1 > 3) return false;
        strncpy(cell1, p, len1);
        return parse_cell(cell1, &cmd->col1, &cmd->row1) &&
               parse_cell(sep + 1, &cmd->col2, &cmd->row2);
    }

    cmd->type = CMD_INVALID;
    return false;
}

// --------------------------------------------------------------------------
// Tarea FreeRTOS — lee UART0, acumula líneas, parsea y encola comandos
// --------------------------------------------------------------------------
static void task_gui_rx(void *pvParams)
{
    (void)pvParams;

    uint8_t buf[GUI_RX_BUF];
    char    line[GUI_LINE_MAX];
    int     line_len = 0;

    while (1) {
        int bytes = uart_read_bytes(UART_GUI_PORT, buf, sizeof(buf) - 1,
                                    pdMS_TO_TICKS(50));
        if (bytes <= 0) continue;

        for (int i = 0; i < bytes; i++) {
            char c = (char)buf[i];

            if (c == '\n') {
                line[line_len] = '\0';

                if (line_len > 0) {
                    asrs_cmd_t cmd;
                    bool ok = parse_line(line, &cmd);

                    if (ok && cmd.type != CMD_INVALID) {
                        // Encolar — si la cola está llena, descartar (no bloquear)
                        if (xQueueSendToBack(cmd_queue, &cmd, 0) != pdTRUE) {
                            LOG_W("GUICOMM", ERR_COMM_TIMEOUT, "cola de comandos llena");
                        }
                    } else {
                        LOG_W("GUICOMM", ERR_COMM_UNKNOWN_CMD, "comando desconocido");
                        // Responder a la GUI con error de parsing
                        gui_comm_send_error(ERR_COMM_UNKNOWN_CMD, 0, 0);
                    }
                }

                line_len = 0;

            } else if (c != '\r') {
                if (line_len < GUI_LINE_MAX - 1) {
                    line[line_len++] = c;
                } else {
                    LOG_W("GUICOMM", ERR_COMM_PARSE, "linea demasiado larga");
                    line_len = 0;
                }
            }
        }
    }
}

// --------------------------------------------------------------------------
// Helpers de formateo de celda (col,row) → "A1"
// --------------------------------------------------------------------------
static void cell_to_str(uint8_t col, uint8_t row, char *out)
{
    out[0] = 'A' + col;
    out[1] = '1' + row;
    out[2] = '\0';
}

// --------------------------------------------------------------------------
// API pública
// --------------------------------------------------------------------------

void gui_comm_init(void)
{
    cmd_queue = xQueueCreate(GUI_CMD_QUEUE, sizeof(asrs_cmd_t));

    // UART0 ya configurado por el sistema (logger usa el mismo puerto).
    // Solo lanzamos la tarea RX.
    xTaskCreate(task_gui_rx, "Task_GUIComm",
                STACK_SIZE_GUICOMM, NULL, PRIORITY_GUICOMM, NULL);

    LOG_I("GUICOMM", ERR_OK, "GUI comm inicializado");
}

bool gui_comm_receive_cmd(asrs_cmd_t *cmd_out)
{
    if (!cmd_queue || !cmd_out) return false;
    return xQueueReceive(cmd_queue, cmd_out, 0) == pdTRUE;
}

void gui_comm_send_ok(const char *op, uint8_t col, uint8_t row, float weight)
{
    char cell[4];
    cell_to_str(col, row, cell);
    char buf[128];
    snprintf(buf, sizeof(buf),
             "{\"status\":\"OK\",\"op\":\"%s\",\"cell\":\"%s\",\"weight\":%.1f}\n",
             op, cell, (double)weight);
    printf("%s", buf);
}

void gui_comm_send_error(asrs_err_t code, uint8_t col, uint8_t row)
{
    char cell[4];
    cell_to_str(col, row, cell);
    // Mapear código a string legible
    const char *code_str = "ERR_UNKNOWN";
    switch (code) {
        case ERR_INVENTORY_CELL_OCCUPIED: code_str = "ERR_CELL_OCCUPIED";    break;
        case ERR_INVENTORY_CELL_EMPTY:    code_str = "ERR_CELL_EMPTY";       break;
        case ERR_WEIGHT_OUT_OF_RANGE:     code_str = "ERR_WEIGHT_OOR";       break;
        case ERR_SYSTEM_INVALID_STATE:    code_str = "ERR_NOT_READY";        break;
        case ERR_COMM_UNKNOWN_CMD:        code_str = "ERR_UNKNOWN_CMD";      break;
        default: break;
    }
    char buf[128];
    snprintf(buf, sizeof(buf),
             "{\"status\":\"ERROR\",\"code\":\"%s\",\"cell\":\"%s\"}\n",
             code_str, cell);
    printf("%s", buf);
}

void gui_comm_send_state(const char *state_str, const float inventory[12])
{
    // Construir JSON del inventario: {"A1":245,"A2":0,...,"D3":100}
    char inv_buf[256];
    int  pos = 0;
    pos += snprintf(inv_buf + pos, sizeof(inv_buf) - pos, "{");
    for (int c = 0; c < MATRIX_COLS; c++) {
        for (int r = 0; r < MATRIX_ROWS; r++) {
            int idx = c * MATRIX_ROWS + r;
            char cell[4];
            cell_to_str((uint8_t)c, (uint8_t)r, cell);
            pos += snprintf(inv_buf + pos, sizeof(inv_buf) - pos,
                            "\"%s\":%.0f", cell, (double)inventory[idx]);
            if (!(c == MATRIX_COLS - 1 && r == MATRIX_ROWS - 1)) {
                pos += snprintf(inv_buf + pos, sizeof(inv_buf) - pos, ",");
            }
        }
    }
    snprintf(inv_buf + pos, sizeof(inv_buf) - pos, "}");

    char buf[320];
    snprintf(buf, sizeof(buf),
             "{\"event\":\"STATE\",\"state\":\"%s\",\"inventory\":%s}\n",
             state_str, inv_buf);
    printf("%s", buf);
}
