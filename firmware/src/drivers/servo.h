#pragma once

#include <stdint.h>
#include "../include/error_codes.h"

void servo_init(void);
asrs_err_t servo_set_angle(uint8_t degrees);
asrs_err_t servo_extend(void);
asrs_err_t servo_retract(void);