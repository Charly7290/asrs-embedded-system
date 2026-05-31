#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/gpio.h"
#include "system.h"
#include "gui_comm.h"
#include "logger.h"
#include "../services/motion.h"
#include "../drivers/stepper.h"
#include "../drivers/servo.h"
#include "../drivers/load_cell.h"
#include "../drivers/switch_uart.h"
#include "../include/config.h"
#include "../include/error_codes.h"

// --------------------------------------------------------------------------
// Estado global del sistema
// --------------------------------------------------------------------------
static volatile sys_state_t sys_state = SYS_BOOT;

// Inventario en RAM: índice = col*3+row, valor = peso en gramos (0 = vacío)
static float inventory[12] = {0};

// --------------------------------------------------------------------------
// Helpers
// --------------------------------------------------------------------------
const char *system_state_str(sys_state_t state)
{
    switch (state) {
        case SYS_BOOT:      return "BOOT";
        case SYS_HOMING:    return "HOMING";
        case SYS_IDLE:      return "IDLE";
        case SYS_OPERATING: return "OPERANDO";
        case SYS_DEGRADED:  return "DEGRADED";
        case SYS_FAULT:     return "FAULT";
        case SYS_ESTOP:     return "E-STOP";
        default:            return "UNKNOWN";
    }
}

sys_state_t system_get_state(void) { return sys_state; }

static void transition_to(sys_state_t new_state)
{
    char msg[48];
    snprintf(msg, sizeof(msg), "%s -> %s",
             system_state_str(sys_state),
             system_state_str(new_state));
    LOG_I("SYSTEM", ERR_OK, msg);
    sys_state = new_state;
    gui_comm_send_state(system_state_str(new_state), inventory);
}

// --------------------------------------------------------------------------
// ISR del E-Stop — solo pone el estado, no hace nada pesado
// --------------------------------------------------------------------------
static void IRAM_ATTR estop_isr_handler(void *arg)
{
    (void)arg;
    // Marcar estado de emergencia — la tarea principal lo procesará
    sys_state = SYS_ESTOP;
}

static void init_estop(void)
{
    gpio_config_t cfg = {
        .pin_bit_mask = (1ULL << ESTOP_GPIO),
        .mode         = GPIO_MODE_INPUT,
        .pull_up_en   = GPIO_PULLUP_ENABLE,
        .pull_down_en = GPIO_PULLDOWN_DISABLE,
        .intr_type    = GPIO_INTR_NEGEDGE,   // flanco de bajada = botón presionado
    };
    gpio_config(&cfg);
    gpio_install_isr_service(0);
    gpio_isr_handler_add(ESTOP_GPIO, estop_isr_handler, NULL);
}

// --------------------------------------------------------------------------
// Reconciliación de inventario
// Compara switches reales vs inventario en RAM y marca DEGRADED si difiere.
// --------------------------------------------------------------------------
static bool reconcile_inventory(void)
{
    if (!switch_uart_has_valid_data()) {
        LOG_W("SYSTEM", ERR_INVENTORY_DISCREPANCY,
              "sin datos validos de switches, omitiendo reconciliacion");
        return true;   // no bloquear el boot si la secundaria no respondió aún
    }

    bool discrepancy = false;
    for (int c = 0; c < MATRIX_COLS; c++) {
        for (int r = 0; r < MATRIX_ROWS; r++) {
            bool sw_occupied  = switch_uart_cell_occupied((uint8_t)c, (uint8_t)r);
            bool nvs_occupied = (inventory[c * MATRIX_ROWS + r] > 5.0f);

            if (sw_occupied != nvs_occupied) {
                char msg[64];
                snprintf(msg, sizeof(msg),
                         "discrepancia celda=%c%d sw=%d nvs=%d",
                         'A' + c, r + 1, sw_occupied, nvs_occupied);
                LOG_W("SYSTEM", ERR_INVENTORY_DISCREPANCY, msg);
                discrepancy = true;
            }
        }
    }
    return !discrepancy;
}

// --------------------------------------------------------------------------
// Ejecución de operaciones
// --------------------------------------------------------------------------
static void execute_store(uint8_t col, uint8_t row)
{
    char cell_str[4] = { 'A' + col, '1' + row, '\0' };
    char msg[48];

    // Verificar que la celda esté vacía
    if (switch_uart_cell_occupied(col, row)) {
        snprintf(msg, sizeof(msg), "STORE %s: celda ocupada", cell_str);
        LOG_W("SYSTEM", ERR_INVENTORY_CELL_OCCUPIED, msg);
        gui_comm_send_error(ERR_INVENTORY_CELL_OCCUPIED, col, row);
        transition_to(SYS_IDLE);
        return;
    }

    // Mover a la celda
    asrs_err_t err = motion_goto_cell(col, row);
    if (err != ERR_OK) {
        LOG_E("SYSTEM", err, "STORE: fallo movimiento");
        gui_comm_send_error(err, col, row);
        transition_to(SYS_FAULT);
        return;
    }

    // Extender servo para depositar
    servo_extend();
    vTaskDelay(pdMS_TO_TICKS(500));   // esperar que el producto quede en la celda

    // Medir peso
    float weight = 0.0f;
    err = load_cell_read_grams(&weight);
    if (err != ERR_OK) {
        LOG_E("SYSTEM", err, "STORE: fallo medicion peso");
        servo_retract();
        gui_comm_send_error(err, col, row);
        transition_to(SYS_FAULT);
        return;
    }

    // Retraer servo
    servo_retract();
    vTaskDelay(pdMS_TO_TICKS(300));

    // Actualizar inventario en RAM
    inventory[col * MATRIX_ROWS + row] = weight;

    snprintf(msg, sizeof(msg), "STORE %s OK peso=%.1fg", cell_str, (double)weight);
    LOG_I("SYSTEM", ERR_OK, msg);
    gui_comm_send_ok("STORE", col, row, weight);
    transition_to(SYS_IDLE);
}

static void execute_retrieve(uint8_t col, uint8_t row)
{
    char cell_str[4] = { 'A' + col, '1' + row, '\0' };
    char msg[48];

    // Verificar que la celda esté ocupada
    if (!switch_uart_cell_occupied(col, row)) {
        snprintf(msg, sizeof(msg), "RETRIEVE %s: celda vacia", cell_str);
        LOG_W("SYSTEM", ERR_INVENTORY_CELL_EMPTY, msg);
        gui_comm_send_error(ERR_INVENTORY_CELL_EMPTY, col, row);
        transition_to(SYS_IDLE);
        return;
    }

    // Mover a la celda
    asrs_err_t err = motion_goto_cell(col, row);
    if (err != ERR_OK) {
        LOG_E("SYSTEM", err, "RETRIEVE: fallo movimiento");
        gui_comm_send_error(err, col, row);
        transition_to(SYS_FAULT);
        return;
    }

    // Extender servo para recoger
    servo_extend();
    vTaskDelay(pdMS_TO_TICKS(500));

    // Medir peso del producto recuperado
    float weight = 0.0f;
    err = load_cell_read_grams(&weight);
    if (err != ERR_OK) {
        LOG_E("SYSTEM", err, "RETRIEVE: fallo medicion peso");
        servo_retract();
        gui_comm_send_error(err, col, row);
        transition_to(SYS_FAULT);
        return;
    }

    servo_retract();
    vTaskDelay(pdMS_TO_TICKS(300));

    // Limpiar inventario
    inventory[col * MATRIX_ROWS + row] = 0.0f;

    snprintf(msg, sizeof(msg), "RETRIEVE %s OK peso=%.1fg", cell_str, (double)weight);
    LOG_I("SYSTEM", ERR_OK, msg);
    gui_comm_send_ok("RETRIEVE", col, row, weight);
    transition_to(SYS_IDLE);
}

// --------------------------------------------------------------------------
// Tarea principal — máquina de estados
// --------------------------------------------------------------------------
static void task_system_control(void *pvParams)
{
    (void)pvParams;

    // ---- BOOT ----
    LOG_I("SYSTEM", ERR_OK, "firmware=" FIRMWARE_VERSION);

    stepper_init();
    servo_init();
    load_cell_init();
    motion_init();
    switch_uart_init();
    gui_comm_init();
    init_estop();

    vTaskDelay(pdMS_TO_TICKS(500));   // dar tiempo a switch_uart de recibir estado inicial

    // ---- HOMING ----
    transition_to(SYS_HOMING);
    asrs_err_t home_err = motion_home();
    if (home_err != ERR_OK) {
        LOG_E("SYSTEM", home_err, "Homing fallido");
        transition_to(SYS_FAULT);
        // En FAULT esperamos RESET de la GUI
    } else {
        // Reconciliación de inventario
        bool ok = reconcile_inventory();
        transition_to(ok ? SYS_IDLE : SYS_DEGRADED);
    }

    // ---- LOOP PRINCIPAL ----
    while (1) {
        // E-Stop tiene prioridad absoluta — la ISR ya actualizó sys_state
        if (sys_state == SYS_ESTOP) {
            LOG_E("SYSTEM", ERR_SYSTEM_ESTOP, "E-STOP activo");
            gui_comm_send_state("E-STOP", inventory);
            // Esperar RESET de GUI
            asrs_cmd_t cmd;
            while (!(gui_comm_receive_cmd(&cmd) && cmd.type == CMD_RESET)) {
                vTaskDelay(pdMS_TO_TICKS(200));
            }
            LOG_I("SYSTEM", ERR_OK, "RESET recibido, iniciando homing");
            transition_to(SYS_HOMING);
            asrs_err_t err = motion_home();
            transition_to(err == ERR_OK ? SYS_IDLE : SYS_FAULT);
            continue;
        }

        // FAULT — esperar RESET
        if (sys_state == SYS_FAULT) {
            asrs_cmd_t cmd;
            if (gui_comm_receive_cmd(&cmd) && cmd.type == CMD_RESET) {
                LOG_I("SYSTEM", ERR_OK, "RESET recibido desde FAULT");
                transition_to(SYS_HOMING);
                asrs_err_t err = motion_home();
                transition_to(err == ERR_OK ? SYS_IDLE : SYS_FAULT);
            }
            vTaskDelay(pdMS_TO_TICKS(200));
            continue;
        }

        // IDLE / DEGRADED — procesar comandos
        if (sys_state == SYS_IDLE || sys_state == SYS_DEGRADED) {
            asrs_cmd_t cmd;
            if (gui_comm_receive_cmd(&cmd)) {
                switch (cmd.type) {
                    case CMD_STORE:
                        transition_to(SYS_OPERATING);
                        execute_store(cmd.col1, cmd.row1);
                        break;

                    case CMD_RETRIEVE:
                        transition_to(SYS_OPERATING);
                        execute_retrieve(cmd.col1, cmd.row1);
                        break;

                    case CMD_SWAP:
                        // SWAP = RETRIEVE de col1/row1 + STORE en col2/row2
                        // Implementación simplificada: secuencial
                        transition_to(SYS_OPERATING);
                        execute_retrieve(cmd.col1, cmd.row1);
                        if (sys_state == SYS_IDLE) {
                            execute_store(cmd.col2, cmd.row2);
                        }
                        break;

                    case CMD_RESET:
                        if (sys_state == SYS_DEGRADED) {
                            LOG_I("SYSTEM", ERR_OK, "operario confirma DEGRADED");
                            transition_to(SYS_IDLE);
                        }
                        break;

                    default:
                        LOG_W("SYSTEM", ERR_COMM_UNKNOWN_CMD, "comando inesperado");
                        gui_comm_send_error(ERR_SYSTEM_INVALID_STATE, 0, 0);
                        break;
                }
            }

            // Enviar estado periódicamente (cada 3 s) para mantener la GUI actualizada
            static TickType_t last_state_tx = 0;
            TickType_t now = xTaskGetTickCount();
            if ((now - last_state_tx) >= pdMS_TO_TICKS(3000)) {
                gui_comm_send_state(system_state_str(sys_state), inventory);
                last_state_tx = now;
            }
        }

        vTaskDelay(pdMS_TO_TICKS(50));   // 50 ms entre ciclos — CPU libre para otras tareas
    }
}

// --------------------------------------------------------------------------
// API pública
// --------------------------------------------------------------------------
void system_init(void)
{
    xTaskCreate(task_system_control, "Task_SysCtrl",
                STACK_SIZE_MOTION, NULL, PRIORITY_MOTION, NULL);
}
