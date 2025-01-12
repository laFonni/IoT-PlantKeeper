#include <stdio.h>
#include <freertos/FreeRTOS.h>
#include <esp_adc/adc_oneshot.h>
#include <driver/gpio.h>

#include "iot_wifi.h"
#include "iot_ble.h"
#include "iot_analog.h"
#include "iot_reconfigure.h"
#include "iot_nvs.h"
#include "iot_mqtt.h"
#include "i2c.h"
#include "topics.h"

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

void app_main(void) {
    iot_nvs_init();
    iot_ble_init();
    iot_wifi_init();
    iot_reconfigure_init(LED_GPIO, BUTTON_GPIO);

    i2c_device_t i2c;
    i2c_init(&i2c, I2C_SDA_PIN, I2C_SCL_PIN, "I2C_TEST");
    get_calibration_params(i2c);

    iot_photoresistor_init(ADC_UNIT, ADC_CHANNEL, ADC_ATTEN);    

    char ssid[64], pass[64];
    get_wifi_credentials_nvs(ssid, pass);
    iot_wifi_start(ssid, pass);

    while (true) {
        if (wifi_connected) {
            trigger_measurment(i2c);
            int32_t result;
            read_temp(i2c, &result);
            int32_t temperature = bmp280_compensate_T_int32(result);
            printf("Temperature: %f\n", temperature / 100.0);
            printf("Light: %ld\n", iot_photoresistor_get(ADC_CHANNEL));
        }

        vTaskDelay(2000 / portTICK_PERIOD_MS);
    }

    return;
}