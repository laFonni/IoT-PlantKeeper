#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <inttypes.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"
#include "nvs_flash.h"
#include "nvs.h"
#include <mqtt_client.h>
#include <esp_bt.h>
#include <esp_wifi.h>

#include <driver/gpio.h>

#include "iot_wifi.h"
#include "iot_ble.h"
#include "iot_nvs.h"
#include "iot_mqtt.h"
#include "i2c.h"
#include "topics.h"

#include "sdkconfig.h"

#define BUTTON_GPIO GPIO_NUM_0
#define LED_GPIO GPIO_NUM_2
#define DEBOUNCE_DELAY_MS 50

#define I2C_SDA_PIN  GPIO_NUM_18
#define I2C_SCL_PIN  GPIO_NUM_19

// char ssid[32], pass[64]
// // ,mqtt_url[64]
// ;
// size_t ssid_len = 32, pass_len = 64
// // , mqtt_url_len = 64
// ;
bool ssid_set = false, pass_set = false, mqtt_url_set = false;

bool setup_mode = false;
static TaskHandle_t blinking_task_handle = NULL;

void iot_reconfigure_init() {
    gpio_reset_pin(LED_GPIO);
    gpio_set_direction(LED_GPIO, GPIO_MODE_OUTPUT);

    gpio_reset_pin(BUTTON_GPIO);
    gpio_set_direction(BUTTON_GPIO, GPIO_MODE_INPUT);
    gpio_set_pull_mode(BUTTON_GPIO, GPIO_PULLUP_ONLY);
}

void blinking_task(void* pvParameters) {
    while (true) {
        gpio_set_level(LED_GPIO, 1);
        vTaskDelay(500 / portTICK_PERIOD_MS);
        gpio_set_level(LED_GPIO, 0);
        vTaskDelay(500 / portTICK_PERIOD_MS);
    }
}

void reconfigure_task(void* pvParameters) {
    while (true) {
        if (gpio_get_level(BUTTON_GPIO) == 0) {
            while (gpio_get_level(BUTTON_GPIO) == 0) {
                vTaskDelay(DEBOUNCE_DELAY_MS / portTICK_PERIOD_MS);
            }

            setup_mode = !setup_mode;
            if (setup_mode) {
                printf("Entering setup mode.\n");
                if (blinking_task_handle == NULL) {
                    xTaskCreate(blinking_task, "blink", 1024, NULL, 1, &blinking_task_handle);
                }

                // iot_mqtt_stop();
                iot_wifi_stop();
                iot_ble_init();
            } else {
                printf("Exiting setup mode.\n");
                if (blinking_task_handle != NULL) {
                    vTaskDelete(blinking_task_handle);
                    blinking_task_handle = NULL;
                    gpio_set_level(LED_GPIO, 0);
                }
                iot_ble_deinit();

                char ssid[64], pass[64];
                size_t ssid_len = 64, pass_len = 64;
                esp_err_t ret;
                nvs_handle_t nvs_handle;
                nvs_open(NVS_NAMESPACE, NVS_READWRITE, &nvs_handle);
                ret = nvs_get_str(nvs_handle, NVS_KEY_SSID, ssid, &ssid_len);
                if (ret == ESP_OK) {
                    ssid_set = true;
                    printf("SSID: %s\n", ssid);
                } else {
                    printf("%s ssid nvs_get_str failed: %s\n", __func__, esp_err_to_name(ret));
                    printf("SSID not set\n");
                }
                ret = nvs_get_str(nvs_handle, NVS_KEY_PASS, pass, &pass_len);
                if (ret == ESP_OK) {
                    pass_set = true;
                    printf("Password: %s\n", pass);
                } else {
                    printf("%s pass nvs_get_str failed: %s\n", __func__, esp_err_to_name(ret));
                    printf("Password not set\n");
                }
                // ret = nvs_get_str(nvs_handle, NVS_KEY_MQTT_URL, mqtt_url, &mqtt_url_len);
                // if (ret == ESP_OK) {
                //     mqtt_url_set = true;

                //     printf("MQTT URL: %s\n", mqtt_url);
                // } else {
                //     printf("%s mqtt url nvs_get_str failed: %s\n", __func__, esp_err_to_name(ret));
                //     printf("MQTT URL not set\n");
                // }
                nvs_close(nvs_handle);

                iot_wifi_start(ssid, pass);
                // iot_mqtt_init(mqtt_url);
            }
        }
        vTaskDelay(DEBOUNCE_DELAY_MS / portTICK_PERIOD_MS);
    }
}

void app_main(void) {
    esp_err_t ret;
    iot_nvs_init();

    ret = esp_bt_controller_mem_release(ESP_BT_MODE_CLASSIC_BT);
    if (ret) {
        printf("%s mem release failed: %s\n", __func__, esp_err_to_name(ret));
        return;
    }

    iot_wifi_init();

    iot_reconfigure_init();
    xTaskCreate(reconfigure_task, "button_task", 4096, NULL, 10, NULL);

    i2c_device_t i2c;
    i2c_init(&i2c, I2C_SDA_PIN, I2C_SCL_PIN, "I2C_TEST");
    get_calibration_params(i2c);


    char ssid[64], pass[64];
    size_t ssid_len = 64, pass_len = 64;
    nvs_handle_t nvs_handle;
    nvs_open(NVS_NAMESPACE, NVS_READWRITE, &nvs_handle);
    ret = nvs_get_str(nvs_handle, NVS_KEY_SSID, ssid, &ssid_len);
    if (ret == ESP_OK) {
        ssid_set = true;
        printf("SSID: %s\n", ssid);
    } else {
        printf("%s ssid nvs_get_str failed: %s\n", __func__, esp_err_to_name(ret));
        printf("SSID not set\n");
    }
    ret = nvs_get_str(nvs_handle, NVS_KEY_PASS, pass, &pass_len);
    if (ret == ESP_OK) {
        pass_set = true;
        printf("Password: %s\n", pass);
    } else {
        printf("%s pass nvs_get_str failed: %s\n", __func__, esp_err_to_name(ret));
        printf("Password not set\n");
    }
    // ret = nvs_get_str(nvs_handle, NVS_KEY_MQTT_URL, mqtt_url, &mqtt_url_len);
    // if (ret == ESP_OK) {
    //     mqtt_url_set = true;

    //     printf("MQTT URL: %s\n", mqtt_url);
    // } else {
    //     printf("%s mqtt url nvs_get_str failed: %s\n", __func__, esp_err_to_name(ret));
    //     printf("MQTT URL not set\n");
    // }
    nvs_close(nvs_handle);

    iot_wifi_start(ssid, pass);
    // iot_mqtt_init(mqtt_url);

    while (true) {
        // if (ssid_set && pass_set && !wifi_connected) {
        //     iot_wifi_init(ssid, pass);
        // }

        // if (wifi_connected && !mqtt_connected) {
        //     iot_mqtt_init(mqtt_url);
        // }

        if (wifi_connected) {
            trigger_measurment(i2c);
            int32_t result;
            read_temp(i2c, &result);
            int32_t temperature = bmp280_compensate_T_int32(result);
            printf("Temperature: %f\n", temperature / 100.0);
        }

        vTaskDelay(2000 / portTICK_PERIOD_MS);
    }

    return;
}