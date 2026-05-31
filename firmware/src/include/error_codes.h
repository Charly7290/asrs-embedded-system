#pragma once

// =============================================================================
// error_codes.h — Códigos de error estructurados del sistema AS/RS
// Patrón: ERR_MÓDULO_CONDICIÓN
// =============================================================================

typedef enum {
    ERR_OK = 0,

    // --- Stepper ---
    ERR_STEPPER_TIMEOUT         = 101,
    ERR_STEPPER_ENDSTOP_MISSING = 102,
    ERR_STEPPER_OUT_OF_BOUNDS   = 103,

    // --- Servo ---
    ERR_SERVO_INIT              = 201,
    ERR_SERVO_TIMEOUT           = 202,

    // --- Celda de carga ---
    ERR_WEIGHT_OUT_OF_RANGE     = 301,
    ERR_WEIGHT_SENSOR_FAIL      = 302,
    ERR_WEIGHT_MISMATCH         = 303,

    // --- Inventario / MCP23017 ---
    ERR_INVENTORY_CELL_OCCUPIED = 401,
    ERR_INVENTORY_CELL_EMPTY    = 402,
    ERR_INVENTORY_DISCREPANCY   = 403,
    ERR_INVENTORY_MCP_FAIL      = 404,

    // --- UART / GUI ---
    ERR_COMM_PARSE              = 501,
    ERR_COMM_UNKNOWN_CMD        = 502,
    ERR_COMM_TIMEOUT            = 503,

    // --- Sistema ---
    ERR_SYSTEM_ESTOP            = 601,
    ERR_SYSTEM_INVALID_STATE    = 602,
    ERR_SYSTEM_NVS_FAIL         = 603,

} asrs_err_t;
