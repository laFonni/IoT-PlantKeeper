#include <string.h>
#include <esp_err.h>
#include <esp_log.h>
#include <nvs.h>
#include <nvs_flash.h>

#include "iot_nvs.h"

void iot_nvs_init(void) {
    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        ESP_ERROR_CHECK(nvs_flash_erase());
        ret = nvs_flash_init();
    }
    ESP_ERROR_CHECK(ret);
}

esp_err_t save_wifi_config_to_nvs(const char* key, const char* value, size_t length) {
    nvs_handle_t nvs_handle;
    esp_err_t err;

    // Open NVS
    err = nvs_open(NVS_NAMESPACE, NVS_READWRITE, &nvs_handle);
    if (err != ESP_OK) {
        ESP_LOGE(NVS_TAG, "Error opening NVS handle: %s", esp_err_to_name(err));
        return err;
    }

    // Write value to NVS
    err = nvs_set_str(nvs_handle, key, value);
    if (err != ESP_OK) {
        ESP_LOGE(NVS_TAG, "Error writing to NVS: %s", esp_err_to_name(err));
        nvs_close(nvs_handle);
        return err;
    }

    // Commit changes
    err = nvs_commit(nvs_handle);
    if (err != ESP_OK) {
        ESP_LOGE(NVS_TAG, "Error committing to NVS: %s", esp_err_to_name(err));
        nvs_close(nvs_handle);
        return err;
    }

    // Close NVS
    nvs_close(nvs_handle);
    ESP_LOGI(NVS_TAG, "Saved %s to NVS", key);
    return ESP_OK;
}

esp_err_t get_wifi_credentials_nvs(char* ssid, char* pass) {
    size_t ssid_len = 64, pass_len = 64;
    nvs_handle_t nvs_handle;
    esp_err_t ret;
    nvs_open(NVS_NAMESPACE, NVS_READWRITE, &nvs_handle);
    ret = nvs_get_str(nvs_handle, NVS_KEY_SSID, ssid, &ssid_len);
    if (ret == ESP_OK) {
        printf("SSID: %s\n", ssid);
    } else {
        printf("%s ssid nvs_get_str failed: %s\n", __func__, esp_err_to_name(ret));
        printf("SSID not set\n");
    }
    ret = nvs_get_str(nvs_handle, NVS_KEY_PASS, pass, &pass_len);
    if (ret == ESP_OK) {
        printf("Password: %s\n", pass);
    } else {
        printf("%s pass nvs_get_str failed: %s\n", __func__, esp_err_to_name(ret));
        printf("Password not set\n");
    }
    nvs_close(nvs_handle);

    return ESP_OK;
}

esp_err_t get_mqtt_uri(char* mqtt_uri) {
    char mqtt_short_uri[64];
    size_t mqtt_short_uri_len = 64;
    esp_err_t ret;
    nvs_handle_t nvs_handle;
    nvs_open(NVS_NAMESPACE, NVS_READWRITE, &nvs_handle);
    ret = nvs_get_str(nvs_handle, NVS_KEY_MQTT_URL, mqtt_short_uri, &mqtt_short_uri_len);
    if (ret == ESP_OK) {
        printf("MQTT URI: %s\n", mqtt_short_uri);
        
        strncpy(mqtt_uri, "mqtt://", strlen("mqtt://") + 1);
        // char mqtt_full_uri[32] = "mqtt://";
        strncat(mqtt_uri, mqtt_short_uri, strlen(mqtt_short_uri) + 1);
        printf("Full MQTT URI: %s\n", mqtt_uri);
    } else {
        printf("%s mqtt url nvs_get_str failed: %s\n", __func__, esp_err_to_name(ret));
        printf("mqtt_url not set\n");
    }
    nvs_close(nvs_handle);
    return ESP_OK;
}