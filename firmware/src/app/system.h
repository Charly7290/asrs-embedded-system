#pragma once

// =============================================================================
// system.h — Máquina de estados principal del AS/RS
//
// Estados: BOOT → HOMING → IDLE → OPERANDO → FAULT / DEGRADED / E-STOP
// =============================================================================

typedef enum {
    SYS_BOOT      = 0,
    SYS_HOMING    = 1,
    SYS_IDLE      = 2,
    SYS_OPERATING = 3,
    SYS_DEGRADED  = 4,
    SYS_FAULT     = 5,
    SYS_ESTOP     = 6,
} sys_state_t;

/**
 * @brief Inicializa todos los módulos y lanza las tareas FreeRTOS.
 *        Llamar desde app_main() una sola vez.
 */
void system_init(void);

/**
 * @brief Retorna el estado actual del sistema.
 */
sys_state_t system_get_state(void);

/**
 * @brief Retorna el nombre del estado como string (para logs y GUI).
 */
const char *system_state_str(sys_state_t state);
