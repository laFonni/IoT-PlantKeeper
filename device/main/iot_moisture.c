#include <driver/gpio.h>

#include "iot_moisture.h"

void iot_soil_moisture_init(gpio_num_t gpio) {
    gpio_set_direction(gpio, GPIO_MODE_INPUT);
    gpio_pullup_dis(gpio);
    gpio_pulldown_dis(gpio);
}

bool is_soil_wet(gpio_num_t gpio) {
    return gpio_get_level(gpio) == 1;
}