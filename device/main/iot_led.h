#pragma once

#include <driver/gpio.h>

void iot_light_init(gpio_num_t gpio);
void iot_light_on(void);
void iot_light_off(void);