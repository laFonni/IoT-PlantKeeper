#include <driver/gpio.h>

#include "iot_moisture.h"

static gpio_num_t soil_moisture_gpio;

void iot_soil_moisture_init(gpio_num_t gpio) {
    gpio_set_direction(gpio, GPIO_MODE_INPUT);
    gpio_pullup_dis(gpio);
    gpio_pulldown_dis(gpio);
    soil_moisture_gpio = gpio;
}

bool is_soil_wet(void) {
    return gpio_get_level(soil_moisture_gpio) == 1;
}