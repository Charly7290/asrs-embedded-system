#pragma once

#include "../include/error_codes.h"

void load_cell_init(void);
asrs_err_t load_cell_read_grams(float *weight_out);