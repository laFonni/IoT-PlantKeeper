#pragma once

#include "driver/gpio.h"

#define I2C_FREQ_HZ    100000    // 100 kHz I2C frequency
#define I2C_DELAY_US   (1000000 / I2C_FREQ_HZ / 2)
#define I2C_SLAVE_ADDR  0x76

typedef struct {
    gpio_num_t sda_pin;
    gpio_num_t scl_pin;
    const char *name;
} i2c_device_t;

typedef struct {
    uint16_t dig_T1;
    int16_t dig_T2;
    int16_t dig_T3;
    uint16_t dig_P1;
    int16_t dig_P2;
    int16_t dig_P3;
    int16_t dig_P4;
    int16_t dig_P5;
    int16_t dig_P6;
    int16_t dig_P7;
    int16_t dig_P8;
    int16_t dig_P9;
} calib_t;

extern calib_t calib;
extern int32_t t_fine;

extern i2c_device_t device;

void i2c_init(i2c_device_t *i2c, gpio_num_t sda, gpio_num_t scl, const char *name);
void i2c_start(i2c_device_t *i2c);
void i2c_stop(i2c_device_t *i2c);
bool i2c_write_byte(i2c_device_t *i2c, uint8_t byte);
uint8_t i2c_read_byte(i2c_device_t *i2c, bool ack);
int get_calibration_params(i2c_device_t i2c);
int trigger_measurment(i2c_device_t i2c);
int read_temp(i2c_device_t i2c, int32_t *result);
int32_t bmp280_compensate_T_int32(int32_t adc_T);
double get_temp_deg(i2c_device_t i2c);