#pragma once

#include <driver/gpio.h>

#define DEBOUNCE_DELAY_MS 50

void iot_reconfigure_init(gpio_num_t led_gpio, gpio_num_t button_gpio);