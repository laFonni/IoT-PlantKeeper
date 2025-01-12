#pragma once

#include <esp_err.h>

#define NVS_NAMESPACE "wifi_config"
#define NVS_KEY_SSID "ssid"
#define NVS_KEY_PASS "password"
#define NVS_KEY_MQTT_URL "mqtt_url"

#define NVS_TAG "NVS"

void iot_nvs_init(void);
esp_err_t save_wifi_config_to_nvs(const char* key, const char* value, size_t length);
esp_err_t get_wifi_credentials_nvs(char* ssid, char* pass);
esp_err_t get_mqtt_uri(char* uri);