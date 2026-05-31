#pragma once

// =============================================================================
// motion.h — Servicio de movimiento del end-effector
//
// Abstrae el posicionamiento del carro sobre la matriz 4×3.
// Calcula pasos necesarios, ejecuta homing con endstops y mueve a celda.
//
// Coordenadas internas:
//   col: 0-3  (A=0, B=1, C=2, D=3)  → eje X (stepper horizontal)
//   row: 0-2  (1=0, 2=1, 3=2)       → eje Y (stepper vertical)
// =============================================================================

#include <stdint.h>
#include "../include/error_codes.h"

/**
 * @brief Inicializa endstops y deja el módulo listo para homing.
 *        Debe llamarse después de stepper_init().
 */
void motion_init(void);

/**
 * @brief Ejecuta la secuencia de homing completa (ambos ejes).
 *        Mueve X e Y hacia sus endstops HOME hasta que se activan.
 *        Al finalizar, la posición interna se registra como (0,0).
 *
 * @return ERR_OK si ambos endstops fueron alcanzados en tiempo.
 *         ERR_STEPPER_ENDSTOP_MISSING si un endstop no se activa en el timeout.
 */
asrs_err_t motion_home(void);

/**
 * @brief Mueve el end-effector a la celda indicada.
 *        Calcula el delta de pasos desde la posición actual.
 *
 * @param col  Columna destino: 0=A, 1=B, 2=C, 3=D
 * @param row  Fila destino:    0=1, 1=2, 2=3
 * @return ERR_OK si el movimiento se completó.
 *         ERR_STEPPER_OUT_OF_BOUNDS si col/row están fuera de rango.
 *         ERR_STEPPER_TIMEOUT si el movimiento excede el tiempo máximo.
 */
asrs_err_t motion_goto_cell(uint8_t col, uint8_t row);

/**
 * @brief Retorna la posición actual del end-effector.
 *
 * @param col_out  Columna actual (0-3)
 * @param row_out  Fila actual (0-2)
 */
void motion_get_position(uint8_t *col_out, uint8_t *row_out);

/**
 * @brief Indica si el homing fue completado exitosamente desde el boot.
 */
bool motion_is_homed(void);
