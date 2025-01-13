#include <driver/gpio.h>

#include "iot_led.h"

static gpio_num_t led_gpio;

void iot_light_init(gpio_num_t gpio) {
    gpio_set_direction(gpio, GPIO_MODE_OUTPUT);
    gpio_pullup_dis(gpio);
    gpio_pulldown_dis(gpio);
    led_gpio = gpio;
}

void iot_light_on(void) {
    gpio_set_level(led_gpio, 1);
}

void iot_light_off(void) {
    gpio_set_level(led_gpio, 0);
}