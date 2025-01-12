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