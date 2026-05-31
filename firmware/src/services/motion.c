#include <stdbool.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/gpio.h"
#include "motion.h"
#include "../drivers/stepper.h"
#include "../include/config.h"
#include "../app/logger.h"

// --------------------------------------------------------------------------
// Parámetros de movimiento
// Ajustar STEP_DELAY_US según velocidad real del motor en el prototipo.
// --------------------------------------------------------------------------
#define STEP_DELAY_US        2000    // 2 ms por paso → ~500 pasos/s
#define HOMING_STEP_DELAY_US 3000    // más lento durante homing (más torque, más seguro)

// Máximo de pasos permitidos durante homing antes de declarar fallo.
// Con STEPS_PER_CELL=200 y 4 columnas, el recorrido máximo es ~800 pasos.
// Se permite 2× de margen para cubrir variaciones mecánicas.
#define HOMING_MAX_STEPS_X   (MATRIX_COLS * STEPS_PER_CELL_X * 2)
#define HOMING_MAX_STEPS_Y   (MATRIX_ROWS * STEPS_PER_CELL_Y * 2)

// Timeout por movimiento a celda: pasos máximos * tiempo por paso * 2 de margen
// Expresado en ticks de FreeRTOS para comparar con xTaskGetTickCount()
#define MOVE_TIMEOUT_MS      15000   // 15 s — cubre A→D o 1→3 con margen

// --------------------------------------------------------------------------
// Estado interno del módulo
// --------------------------------------------------------------------------
static uint8_t current_col = 0;
static uint8_t current_row = 0;
static bool    homed        = false;

// --------------------------------------------------------------------------
// Helpers de endstop
// --------------------------------------------------------------------------

/** Configura los 4 endstops como entradas con pull-up (activos en bajo). */
static void init_endstops(void)
{
    const gpio_num_t pins[] = {
        ENDSTOP_X_HOME, ENDSTOP_X_LIMIT,
        ENDSTOP_Y_HOME, ENDSTOP_Y_LIMIT,
    };
    for (int i = 0; i < 4; i++) {
        gpio_config_t cfg = {
            .pin_bit_mask = (1ULL << pins[i]),
            .mode         = GPIO_MODE_INPUT,
            .pull_up_en   = GPIO_PULLUP_DISABLE,
            .pull_down_en = GPIO_PULLDOWN_DISABLE,
            .intr_type    = GPIO_INTR_DISABLE,
        };
        gpio_config(&cfg);
    }
}

/** Retorna true si el endstop indicado está activado (nivel LOW). */
static inline bool endstop_triggered(gpio_num_t pin)
{
    return gpio_get_level(pin) == 0;
}

// --------------------------------------------------------------------------
// Homing de un eje
//
// Mueve el stepper en DIR_BACKWARD (hacia home) paso a paso.
// Se detiene cuando el endstop se activa o se alcanza max_steps.
// --------------------------------------------------------------------------
static asrs_err_t home_axis(stepper_id_t motor,
                             gpio_num_t   endstop_pin,
                             int          max_steps,
                             const char  *axis_name)
{
    char msg[48];

    snprintf(msg, sizeof(msg), "homing %s...", axis_name);
    LOG_I("MOTION", ERR_OK, msg);

    for (int i = 0; i < max_steps; i++) {
        if (endstop_triggered(endstop_pin)) {
            snprintf(msg, sizeof(msg), "%s home OK en %d pasos", axis_name, i);
            LOG_I("MOTION", ERR_OK, msg);
            return ERR_OK;
        }
        stepper_move(motor, 1, DIR_BACKWARD, HOMING_STEP_DELAY_US);
    }

    // Llegamos al límite de pasos sin encontrar el endstop
    snprintf(msg, sizeof(msg), "%s endstop no detectado en %d pasos", axis_name, max_steps);
    LOG_E("MOTION", ERR_STEPPER_ENDSTOP_MISSING, msg);
    return ERR_STEPPER_ENDSTOP_MISSING;
}

// --------------------------------------------------------------------------
// API pública
// --------------------------------------------------------------------------

void motion_init(void)
{
    init_endstops();
    homed       = false;
    current_col = 0;
    current_row = 0;
    LOG_I("MOTION", ERR_OK, "Motion inicializado");
}

asrs_err_t motion_home(void)
{
    asrs_err_t err;

    // Primero eje X, luego eje Y — orden importante para evitar colisiones
    err = home_axis(STEPPER_X, ENDSTOP_X_HOME, HOMING_MAX_STEPS_X, "X");
    if (err != ERR_OK) return err;

    err = home_axis(STEPPER_Y, ENDSTOP_Y_HOME, HOMING_MAX_STEPS_Y, "Y");
    if (err != ERR_OK) return err;

    current_col = 0;
    current_row = 0;
    homed       = true;

    LOG_I("MOTION", ERR_OK, "Homing completado. Posicion=(0,0)");
    return ERR_OK;
}

asrs_err_t motion_goto_cell(uint8_t col, uint8_t row)
{
    if (col >= MATRIX_COLS || row >= MATRIX_ROWS) {
        char msg[48];
        snprintf(msg, sizeof(msg), "celda fuera de rango col=%d row=%d", col, row);
        LOG_E("MOTION", ERR_STEPPER_OUT_OF_BOUNDS, msg);
        return ERR_STEPPER_OUT_OF_BOUNDS;
    }

    // Calcular delta de pasos
    int delta_x = (int)col - (int)current_col;
    int delta_y = (int)row - (int)current_row;

    char msg[64];
    snprintf(msg, sizeof(msg), "goto col=%d row=%d dx=%d dy=%d",
             col, row, delta_x, delta_y);
    LOG_I("MOTION", ERR_OK, msg);

    // Mover X primero, luego Y — evita movimientos diagonales sin control
    if (delta_x != 0) {
        stepper_dir_t dir = (delta_x > 0) ? DIR_FORWARD : DIR_BACKWARD;
        int steps = (delta_x > 0) ? delta_x : -delta_x;
        stepper_move(STEPPER_X, steps * STEPS_PER_CELL_X, dir, STEP_DELAY_US);
    }

    if (delta_y != 0) {
        stepper_dir_t dir = (delta_y > 0) ? DIR_FORWARD : DIR_BACKWARD;
        int steps = (delta_y > 0) ? delta_y : -delta_y;
        stepper_move(STEPPER_Y, steps * STEPS_PER_CELL_Y, dir, STEP_DELAY_US);
    }

    current_col = col;
    current_row = row;

    snprintf(msg, sizeof(msg), "posicion actualizada col=%d row=%d", col, row);
    LOG_I("MOTION", ERR_OK, msg);
    return ERR_OK;
}

void motion_get_position(uint8_t *col_out, uint8_t *row_out)
{
    if (col_out) *col_out = current_col;
    if (row_out) *row_out = current_row;
}

bool motion_is_homed(void)
{
    return homed;
}
