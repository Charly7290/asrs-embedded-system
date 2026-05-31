#pragma once

// =============================================================================
// config.h — Configuración global del sistema AS/RS
// Editar este archivo para ajustar pines y parámetros del sistema
// =============================================================================

// --- Firmware ---
#define FIRMWARE_VERSION     "v1.0.0"

// --- UART GUI ---
#define UART_GUI_PORT        UART_NUM_0
#define UART_GUI_BAUDRATE    115200
#define UART_GUI_TX          GPIO_NUM_1
#define UART_GUI_RX          GPIO_NUM_3

// Stepper X (L298N #1)
#define STEPPER_X_IN1        GPIO_NUM_16
#define STEPPER_X_IN2        GPIO_NUM_17
#define STEPPER_X_IN3        GPIO_NUM_18
#define STEPPER_X_IN4        GPIO_NUM_19

// Stepper Y (L298N #2)
#define STEPPER_Y_IN1        GPIO_NUM_25
#define STEPPER_Y_IN2        GPIO_NUM_26
#define STEPPER_Y_IN3        GPIO_NUM_27
#define STEPPER_Y_IN4        GPIO_NUM_14

// Servo SG-5010
#define SERVO_GPIO           GPIO_NUM_13
#define SERVO_PWM_CHANNEL    0
#define SERVO_FREQ_HZ        50
#define SERVO_MIN_US         500
#define SERVO_MAX_US         2500
#define SERVO_RETRACTED_DEG  0
#define SERVO_EXTENDED_DEG   90

// Celda de carga
#define LOAD_CELL_ADC_CH     ADC1_CHANNEL_4   // GPIO 32
#define LOAD_CELL_SAMPLES    64
#define LOAD_CELL_FILTER     10
#define WEIGHT_MIN_G         10.0f
#define WEIGHT_MAX_G         1000.0f

// UART switches (ESP32 secundaria)
#define UART_SW_PORT         UART_NUM_1
#define UART_SW_BAUDRATE     115200
#define UART_SW_RX           GPIO_NUM_21   // recibe datos de la ESP32 secundaria
#define UART_SW_TX           GPIO_NUM_22   // por si necesitamos enviar comandos a la secundaria (opcional)

// Endstops
#define ENDSTOP_X_HOME       GPIO_NUM_34
#define ENDSTOP_X_LIMIT      GPIO_NUM_35
#define ENDSTOP_Y_HOME       GPIO_NUM_36
#define ENDSTOP_Y_LIMIT      GPIO_NUM_39

// Sensor IR
#define IR_SENSOR_GPIO       GPIO_NUM_33

// E-Stop
#define ESTOP_GPIO           GPIO_NUM_23

// Luces piloto (relés)
#define LIGHT_RED_GPIO       GPIO_NUM_2
#define LIGHT_GREEN_GPIO     GPIO_NUM_5
#define LIGHT_YELLOW_GPIO    GPIO_NUM_15

// Matriz
#define MATRIX_COLS          4
#define MATRIX_ROWS          3

// Pasos por celda 
#define STEPS_PER_CELL_X     200
#define STEPS_PER_CELL_Y     200

// FreeRTOS stacks y prioridades
#define STACK_SIZE_MOTION    4096
#define STACK_SIZE_GUICOMM   4096
#define STACK_SIZE_INVENTORY 2048
#define STACK_SIZE_LOGGER    4096

#define PRIORITY_MOTION      5
#define PRIORITY_SAFETY      6
#define PRIORITY_GUICOMM     3
#define PRIORITY_INVENTORY   3
#define PRIORITY_LOGGER      2