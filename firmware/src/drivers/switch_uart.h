#pragma once

// =============================================================================
// switch_uart.h — Receptor de estado de switches desde la ESP32 secundaria
//
// Escucha por UART1 los frames "SW:XXXXXXXXXXXX\n" que envía la secundaria
// cada vez que cambia el estado de algún microswitch.
//
// Expone el estado actualizado al resto del firmware (inventory.c, system.c)
// con la misma interfaz conceptual que tenía el MCP23017.
// =============================================================================

#include <stdbool.h>
#include <stdint.h>
#include "../include/error_codes.h"

// --------------------------------------------------------------------------
// Inicialización
// --------------------------------------------------------------------------

/**
 * @brief Inicializa UART1 y lanza la tarea de recepción.
 *        Debe llamarse durante el boot, antes de acceder al inventario.
 */
void switch_uart_init(void);

// --------------------------------------------------------------------------
// Lectura de estado (thread-safe, se puede llamar desde cualquier tarea)
// --------------------------------------------------------------------------

/**
 * @brief Devuelve el último estado conocido de los 12 switches como bitmask.
 *        Bit 0 = A1, bit 11 = C4.  '1' = celda OCUPADA.
 *
 * @return uint16_t  Bitmask de 12 bits.
 */
uint16_t switch_uart_get_state(void);

/**
 * @brief Consulta si una celda específica está ocupada.
 *
 * @param col  Columna: 0 = A, 1 = B, 2 = C
 * @param row  Fila:    0 = 1, 1 = 2, 2 = 3, 3 = 4
 * @return true si ocupada, false si vacía o índices inválidos.
 */
bool switch_uart_cell_occupied(uint8_t col, uint8_t row);

/**
 * @brief Indica si ya se recibió al menos un frame válido desde el arranque.
 *        Usar para saber si el estado es confiable antes de operar.
 *
 * @return true si hay datos válidos.
 */
bool switch_uart_has_valid_data(void);
