#pragma once

#include <stdint.h>
#include "../include/error_codes.h"

typedef enum {
    STEPPER_X = 0,
    STEPPER_Y = 1,
} stepper_id_t;

typedef enum {
    DIR_FORWARD  = 1,
    DIR_BACKWARD = -1,
} stepper_dir_t;

void stepper_init(void);
asrs_err_t stepper_move(stepper_id_t id, int steps, stepper_dir_t dir, uint32_t step_delay_us);
