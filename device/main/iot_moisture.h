#pragma once

#include <driver/gpio.h>

void iot_soil_moisture_init(gpio_num_t gpio);
bool is_soil_wet(void);