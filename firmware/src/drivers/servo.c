#include "driver/ledc.h"
#include "servo.h"
#include "../include/config.h"
#include "../app/logger.h"

#define LEDC_TIMER      LEDC_TIMER_0
#define LEDC_MODE       LEDC_LOW_SPEED_MODE
#define LEDC_RESOLUTION LEDC_TIMER_13_BIT
#define LEDC_MAX_DUTY   8191

static uint32_t angle_to_duty(uint8_t degrees) {
    uint32_t us = SERVO_MIN_US + ((uint32_t)degrees * (SERVO_MAX_US - SERVO_MIN_US)) / 180;
    return (us * LEDC_MAX_DUTY) / 20000;
}

void servo_init(void) {
    ledc_timer_config_t timer = {
        .speed_mode      = LEDC_MODE,
        .timer_num       = LEDC_TIMER,
        .duty_resolution = LEDC_RESOLUTION,
        .freq_hz         = SERVO_FREQ_HZ,
        .clk_cfg         = LEDC_AUTO_CLK,
    };
    ledc_timer_config(&timer);

    ledc_channel_config_t channel = {
        .speed_mode = LEDC_MODE,
        .channel    = SERVO_PWM_CHANNEL,
        .timer_sel  = LEDC_TIMER,
        .intr_type  = LEDC_INTR_DISABLE,
        .gpio_num   = SERVO_GPIO,
        .duty       = angle_to_duty(SERVO_RETRACTED_DEG),
        .hpoint     = 0,
    };
    ledc_channel_config(&channel);
    LOG_I("SERVO", ERR_OK, "Servo inicializado");
}

asrs_err_t servo_set_angle(uint8_t degrees) {
    if (degrees > 180) {
        LOG_E("SERVO", ERR_SERVO_INIT, "Angulo fuera de rango");
        return ERR_SERVO_INIT;
    }
    ledc_set_duty(LEDC_MODE, SERVO_PWM_CHANNEL, angle_to_duty(degrees));
    ledc_update_duty(LEDC_MODE, SERVO_PWM_CHANNEL);
    return ERR_OK;
}

asrs_err_t servo_extend(void) {
    LOG_I("SERVO", ERR_OK, "Extendiendo");
    return servo_set_angle(SERVO_EXTENDED_DEG);
}

asrs_err_t servo_retract(void) {
    LOG_I("SERVO", ERR_OK, "Retrayendo");
    return servo_set_angle(SERVO_RETRACTED_DEG);
}