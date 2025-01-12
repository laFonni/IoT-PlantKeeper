#include "freertos/FreeRTOS.h"
#include <driver/gpio.h>

#include "iot_wifi.h"
#include "iot_ble.h"
#include "iot_nvs.h"

#include "iot_reconfigure.h"

static bool setup_mode = false;
static gpio_num_t led_gpio;
static gpio_num_t button_gpio;
static TaskHandle_t blinking_task_handle = NULL;

static void blinking_task(void* pvParameters);
static void reconfigure_task(void* pvParameters);

void iot_reconfigure_init(gpio_num_t led, gpio_num_t button) {
    led_gpio = led;
    button_gpio = button;
    gpio_reset_pin(led_gpio);
    gpio_set_direction(led_gpio, GPIO_MODE_OUTPUT);

    gpio_reset_pin(button_gpio);
    gpio_set_direction(button_gpio, GPIO_MODE_INPUT);
    gpio_set_pull_mode(button_gpio, GPIO_PULLUP_ONLY);

    xTaskCreate(reconfigure_task, "button_task", 4096, NULL, 10, NULL);
}

static void blinking_task(void* pvParameters) {
    while (true) {
        gpio_set_level(led_gpio, 1);
        vTaskDelay(500 / portTICK_PERIOD_MS);
        gpio_set_level(led_gpio, 0);
        vTaskDelay(500 / portTICK_PERIOD_MS);
    }
}

static void reconfigure_task(void* pvParameters) {
    while (true) {
        if (gpio_get_level(button_gpio) == 0) {
            while (gpio_get_level(button_gpio) == 0) {
                vTaskDelay(DEBOUNCE_DELAY_MS / portTICK_PERIOD_MS);
            }

            setup_mode = !setup_mode;
            if (setup_mode) {
                printf("Entering setup mode.\n");
                if (blinking_task_handle == NULL) {
                    xTaskCreate(blinking_task, "blink", 1024, NULL, 1, &blinking_task_handle);
                }

                iot_wifi_stop();
                iot_ble_start();
            } else {
                printf("Exiting setup mode.\n");
                if (blinking_task_handle != NULL) {
                    vTaskDelete(blinking_task_handle);
                    blinking_task_handle = NULL;
                    gpio_set_level(led_gpio, 0);
                }

                iot_ble_stop();

                char ssid[64], pass[64];
                get_wifi_credentials_nvs(ssid, pass);
                iot_wifi_start(ssid, pass);
            }
        }
        vTaskDelay(DEBOUNCE_DELAY_MS / portTICK_PERIOD_MS);
    }
}