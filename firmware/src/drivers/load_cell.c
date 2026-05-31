#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/adc.h"
#include "load_cell.h"
#include "../include/config.h"
#include "../app/logger.h"

#define CAL_POINTS 5
static const float cal_raw[CAL_POINTS] = {298.0f, 309.0f, 460.0f, 1163.0f, 2805.0f};
static const float cal_g[CAL_POINTS]   = {0.0f,   100.0f, 200.0f, 500.0f,  1000.0f};

static float filter_buffer[LOAD_CELL_FILTER];
static int   filter_index = 0;

static float read_average_raw(void) {
    long sum = 0;
    for (int i = 0; i < LOAD_CELL_SAMPLES; i++) {
        sum += adc1_get_raw(LOAD_CELL_ADC_CH);
        vTaskDelay(pdMS_TO_TICKS(5));
    }
    return (float)sum / LOAD_CELL_SAMPLES;
}

static float apply_filter(float raw) {
    filter_buffer[filter_index] = raw;
    filter_index = (filter_index + 1) % LOAD_CELL_FILTER;
    float total = 0;
    for (int i = 0; i < LOAD_CELL_FILTER; i++)
        total += filter_buffer[i];
    return total / LOAD_CELL_FILTER;
}

static float raw_to_grams(float raw) {
    if (raw <= cal_raw[0]) return 0.0f;
    if (raw >= cal_raw[CAL_POINTS - 1]) return cal_g[CAL_POINTS - 1];
    for (int i = 0; i < CAL_POINTS - 1; i++) {
        if (raw >= cal_raw[i] && raw <= cal_raw[i + 1]) {
            float t = (raw - cal_raw[i]) / (cal_raw[i + 1] - cal_raw[i]);
            return cal_g[i] + t * (cal_g[i + 1] - cal_g[i]);
        }
    }
    return 0.0f;
}

void load_cell_init(void) {
    adc1_config_width(ADC_WIDTH_BIT_12);
    adc1_config_channel_atten(LOAD_CELL_ADC_CH, ADC_ATTEN_DB_12);
    for (int i = 0; i < LOAD_CELL_FILTER; i++) {
        apply_filter(read_average_raw());
        vTaskDelay(pdMS_TO_TICKS(100));
    }
    LOG_I("WEIGHT", ERR_OK, "Celda de carga inicializada");
}

asrs_err_t load_cell_read_grams(float *weight_out) {
    if (!weight_out) return ERR_WEIGHT_SENSOR_FAIL;
    float raw   = read_average_raw();
    float filt  = apply_filter(raw);
    float grams = raw_to_grams(filt);
    char msg[64];
    if (grams > WEIGHT_MAX_G) {
        snprintf(msg, sizeof(msg), "peso=%.1fg > maximo=%.0fg", grams, WEIGHT_MAX_G);
        LOG_E("WEIGHT", ERR_WEIGHT_OUT_OF_RANGE, msg);
        *weight_out = grams;
        return ERR_WEIGHT_OUT_OF_RANGE;
    }
    *weight_out = grams;
    return ERR_OK;
}