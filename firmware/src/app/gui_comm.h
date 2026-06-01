#pragma once

// =============================================================================
// gui_comm.h — Comunicación bidireccional con la GUI Python/tkinter
//
// Recibe comandos ASCII por UART0 (USB-Serial) terminados en '\n'.
// Responde y envía eventos en JSON terminados en '\n'.
//
// Comandos entrantes:
//   STORE:XN\n       → almacenar en celda (ej: STORE:A1)
//   RETRIEVE:XN\n    → recuperar de celda (ej: RETRIEVE:B3)
//   SWAP:XN:XN\n     → intercambiar dos celdas (ej: SWAP:A1:C3)
//   RESET\n          → resetear desde FAULT o E-STOP
//
// Respuestas salientes (JSON + '\n'):
//   {"status":"OK","op":"STORE","cell":"A1","weight":245.3}
//   {"status":"ERROR","code":"ERR_CELL_OCCUPIED","cell":"A1"}
//   {"event":"STATE","state":"IDLE","inventory":{"A1":245,"B3":0,...}}
// =============================================================================

#include <stdint.h>
#include <stdbool.h>
#include "../include/error_codes.h"

// Tipos de comando reconocidos
typedef enum {
    CMD_NONE     = 0,
    CMD_STORE    = 1,
    CMD_RETRIEVE = 2,
    CMD_SWAP     = 3,
    CMD_RESET    = 4,
    CMD_INVALID  = 5,
} cmd_type_t;

// Estructura de comando parseado
typedef struct {
    cmd_type_t type;
    uint8_t    col1;   // columna celda 1 (0-3)
    uint8_t    row1;   // fila celda 1 (0-2)
    uint8_t    col2;   // columna celda 2 (solo para SWAP)
    uint8_t    row2;   // fila celda 2 (solo para SWAP)
} asrs_cmd_t;

/**
 * @brief Inicializa el módulo. Llama solo una vez en el boot.
 *        UART0 ya debe estar configurado por logger_init().
 */
void gui_comm_init(void);

/**
 * @brief Intenta leer y parsear un comando de la cola RX.
 *        No bloqueante — retorna CMD_NONE si no hay comando disponible.
 *
 * @param cmd_out  Estructura de comando resultante.
 * @return true si se recibió un comando válido o inválido (cmd_out válido).
 *         false si no hay nada en la cola.
 */
bool gui_comm_receive_cmd(asrs_cmd_t *cmd_out);

/**
 * @brief Envía respuesta OK de una operación completada.
 */
void gui_comm_send_ok(const char *op, uint8_t col, uint8_t row, float weight);

/**
 * @brief Envía respuesta de error.
 */
void gui_comm_send_error(asrs_err_t code, uint8_t col, uint8_t row);

/**
 * @brief Envía el estado completo del sistema (estado + inventario).
 *        Llamar periódicamente desde IDLE o tras cada operación.
 *
 * @param state_str   Nombre del estado actual ("IDLE", "OPERANDO", etc.)
 * @param inventory   Array de 12 pesos en gramos (índice = col*3+row).
 *                    0 = celda vacía.
 */
void gui_comm_send_state(const char *state_str, const float inventory[12]);
