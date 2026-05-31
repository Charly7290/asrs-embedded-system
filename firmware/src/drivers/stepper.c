#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/gpio.h"
#include "esp_timer.h"
#include "stepper.h"
#include "../include/config.h"
#include "../app/logger.h"

static const uint8_t HALF_STEP[8][4] = {
    {1, 0, 0, 0},
    {1, 1, 0, 0},
    {0, 1, 0, 0},
    {0, 1, 1, 0},
    {0, 0, 1, 0},
    {0, 0, 1, 1},
    {0, 0, 0, 1},
    {1, 0, 0, 1},
};

typedef struct {
    gpio_num_t in1, in2, in3, in4;
    int        step_index;
} stepper_t;

static stepper_t steppers[2] = {
    { STEPPER_X_IN1, STEPPER_X_IN2, STEPPER_X_IN3, STEPPER_X_IN4, 0 },
    { STEPPER_Y_IN1, STEPPER_Y_IN2, STEPPER_Y_IN3, STEPPER_Y_IN4, 0 },
};

static void set_pins(stepper_t *s, int idx) {
    gpio_set_level(s->in1, HALF_STEP[idx][0]);
    gpio_set_level(s->in2, HALF_STEP[idx][1]);
    gpio_set_level(s->in3, HALF_STEP[idx][2]);
    gpio_set_level(s->in4, HALF_STEP[idx][3]);
}

static void config_output(gpio_num_t pin) {
    gpio_config_t cfg = {
        .pin_bit_mask = (1ULL << pin),
        .mode         = GPIO_MODE_OUTPUT,
        .pull_up_en   = GPIO_PULLUP_DISABLE,
        .pull_down_en = GPIO_PULLDOWN_DISABLE,
        .intr_type    = GPIO_INTR_DISABLE,
    };
    gpio_config(&cfg);
}

void stepper_init(void) {
    for (int i = 0; i < 2; i++) {
        config_output(steppers[i].in1);
        config_output(steppers[i].in2);
        config_output(steppers[i].in3);
        config_output(steppers[i].in4);
    }
    LOG_I("STEPPER", ERR_OK, "Steppers inicializados");
}

asrs_err_t stepper_move(stepper_id_t id, int steps, stepper_dir_t dir, uint32_t step_delay_us) {
    stepper_t *s = &steppers[id];
    for (int i = 0; i < steps; i++) {
        s->step_index = (s->step_index + dir + 8) % 8;
        set_pins(s, s->step_index);
        esp_rom_delay_us(step_delay_us);
    }
    return ERR_OK;
}