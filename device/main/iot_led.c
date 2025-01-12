#include <driver/gpio.h>

#include "iot_led.h"

void iot_light_init(gpio_num_t gpio) {
    gpio_set_direction(gpio, GPIO_MODE_OUTPUT);
    gpio_pullup_dis(gpio);
    gpio_pulldown_dis(gpio);
}