#include <stdio.h>
#include <freertos/FreeRTOS.h>
#include <esp_adc/adc_oneshot.h>
#include <driver/gpio.h>

#include "iot_wifi.h"
#include "iot_moisture.h"
#include "iot_ble.h"
#include "iot_analog.h"
#include "iot_led.h"
#include "iot_reconfigure.h"
#include "iot_nvs.h"
#include "iot_mqtt.h"
#include "i2c.h"
#include "iot_time.h"
#include "iot_mac.h"

// Setup mode config
#define BUTTON_GPIO GPIO_NUM_0
#define LED_GPIO GPIO_NUM_2

// Temperature sensor config
#define I2C_SDA_PIN  GPIO_NUM_18
#define I2C_SCL_PIN  GPIO_NUM_19

// Photoresistor config
#define ADC_CHANNEL        ADC_CHANNEL_3   // GPIO36 (ADC1 Channel 0)
#define ADC_ATTEN          ADC_ATTEN_DB_12 // Maximum input voltage ~3.6V
#define ADC_UNIT           ADC_UNIT_1      // Use ADC1

// Moisture sensor config
#define SOIL_GPIO GPIO_NUM_5

// Light config
#define LIGHT_GPIO GPIO_NUM_21

void app_main(void) {
    iot_nvs_init();
    iot_ble_init();
    iot_wifi_init();
    iot_reconfigure_init(LED_GPIO, BUTTON_GPIO);

    get_mac_address();

    i2c_device_t i2c;
    i2c_init(&i2c, I2C_SDA_PIN, I2C_SCL_PIN, "I2C_TEST");
    get_calibration_params(i2c);
    device = i2c;

    iot_photoresistor_init(ADC_UNIT, ADC_CHANNEL, ADC_ATTEN);
    iot_soil_moisture_init(SOIL_GPIO);  
    iot_light_init(LIGHT_GPIO);

    initialize_sntp();

    char ssid[64], pass[64];
    get_wifi_credentials_nvs(ssid, pass);
    iot_wifi_start(ssid, pass);

    // while (true) {
    //     if (wifi_connected && time_synchronized) {
    //         char timestamp[64];
    //         get_timestamp(timestamp);
    //         printf("Timestamp: %s\n", timestamp);
    //         printf("Temperature: %f\n", get_temp_deg(i2c));
    //         printf("Light: %ld\n", iot_photoresistor_get());
    //         printf("Is wet: %d\n", is_soil_wet());

    //         gpio_set_level(LIGHT_GPIO, 1);
    //         vTaskDelay(500 / portTICK_PERIOD_MS);
    //         gpio_set_level(LIGHT_GPIO, 0);
    //         vTaskDelay(500 / portTICK_PERIOD_MS);
    //     }

    //     vTaskDelay(2000 / portTICK_PERIOD_MS);
    // }
}