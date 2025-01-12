#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <inttypes.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"
#include "esp_system.h"
#include "esp_log.h"
#include "nvs_flash.h"
#include "esp_bt.h"
#include "nvs.h"
#include "esp_wifi.h"
#include "esp_http_client.h"
#include "mqtt_client.h"

#include "esp_gap_ble_api.h"
#include "esp_gatts_api.h"
#include "esp_bt_defs.h"
#include "esp_bt_main.h"
#include "esp_bt_device.h"
#include "esp_gatt_common_api.h"

#include "sdkconfig.h"
#include "topics.h"
#include "cJSON.h"
#include <time.h>
#include <sys/time.h>

#define GATTS_TAG "GATTS_DEMO"
#define CONFIG_BROKER_URL "mqtt://192.168.203.14:1883"
#define WIFI_SUCCESS 1
#define LED_GPIO 2

#define I2C_SDA_PIN  GPIO_NUM_18
#define I2C_SCL_PIN  GPIO_NUM_19

// BLE Configuration
#define NVS_NAMESPACE "wifi_config"
#define NVS_KEY_SSID "ssid"
#define NVS_KEY_PASS "password"
#define WIFI_CONFIG_SERVICE_UUID_128 "WIFI_CONFIG"
#define WIFI_SSID_CHAR_UUID_128    "SET_SSID"
#define WIFI_PASS_CHAR_UUID_128    "SET_PASSWORD"
#define PROFILE_NUM 1
#define PROFILE_WIFI_APP_ID 0
#define GATTS_NUM_HANDLE_WIFI     6

// Global variables
static bool wifi_connected = false;
static bool mqtt_connected = false;
static EventGroupHandle_t wifi_event_group;
static esp_event_handler_instance_t wifi_handler_event_instance;
static esp_event_handler_instance_t got_ip_event_instance;
static esp_mqtt_client_handle_t mqtt_client;
static TaskHandle_t mqtt_publish_task_handle;

// BLE related variables
static uint8_t adv_config_done = 0;
#define adv_config_flag      (1 << 0)
#define scan_rsp_config_flag (1 << 1)

static uint8_t adv_service_uuid128[32] = {
    0xfb, 0x34, 0x9b, 0x5f, 0x80, 0x00, 0x00, 0x80, 0x00, 0x10, 0x00, 0x00, 0xEE, 0x00, 0x00, 0x00,
    0xfb, 0x34, 0x9b, 0x5f, 0x80, 0x00, 0x00, 0x80, 0x00, 0x10, 0x00, 0x00, 0xFF, 0x00, 0x00, 0x00,
};

static esp_ble_adv_data_t adv_data = {
    .set_scan_rsp = false,
    .include_name = true,
    .include_txpower = false,
    .min_interval = 0x0006,
    .max_interval = 0x0010,
    .appearance = 0x00,
    .manufacturer_len = 0,
    .p_manufacturer_data =  NULL,
    .service_data_len = 0,
    .p_service_data = NULL,
    .service_uuid_len = sizeof(adv_service_uuid128),
    .p_service_uuid = adv_service_uuid128,
    .flag = (ESP_BLE_ADV_FLAG_GEN_DISC | ESP_BLE_ADV_FLAG_BREDR_NOT_SPT),
};

static esp_ble_adv_data_t scan_rsp_data = {
    .set_scan_rsp = true,
    .include_name = true,
    .include_txpower = true,
    .appearance = 0x00,
    .manufacturer_len = 0,
    .p_manufacturer_data =  NULL,
    .service_data_len = 0,
    .p_service_data = NULL,
    .service_uuid_len = sizeof(adv_service_uuid128),
    .p_service_uuid = adv_service_uuid128,
    .flag = (ESP_BLE_ADV_FLAG_GEN_DISC | ESP_BLE_ADV_FLAG_BREDR_NOT_SPT),
};

static esp_ble_adv_params_t adv_params = {
    .adv_int_min        = 0x20,
    .adv_int_max        = 0x40,
    .adv_type           = ADV_TYPE_IND,
    .own_addr_type      = BLE_ADDR_TYPE_PUBLIC,
    .channel_map        = ADV_CHNL_ALL,
    .adv_filter_policy = ADV_FILTER_ALLOW_SCAN_ANY_CON_ANY,
};

struct gatts_profile_inst {
    esp_gatts_cb_t gatts_cb;
    uint16_t gatts_if;
    uint16_t app_id;
    uint16_t conn_id;
    uint16_t service_handle;
    esp_gatt_srvc_id_t service_id;
    uint16_t ssid_char_handle;
    uint16_t pass_char_handle;
    uint16_t char_handle;
    esp_bt_uuid_t char_uuid;
    esp_gatt_perm_t perm;
    esp_gatt_char_prop_t property;
    uint16_t descr_handle;
    esp_bt_uuid_t descr_uuid;
};

static struct gatts_profile_inst gl_profile_tab[PROFILE_NUM] = {
    [PROFILE_WIFI_APP_ID] = {
        .gatts_cb = gatts_profile_wifi_event_handler,
        .gatts_if = ESP_GATT_IF_NONE,
    },
};

// Function declarations
static void wifi_event_handler(void* arg, esp_event_base_t event_base, int32_t event_id, void* event_data);
static void ip_event_handler(void* arg, esp_event_base_t event_base, int32_t event_id, void* event_data);
static void mqtt_event_handler(void *handler_args, esp_event_base_t base, int32_t event_id, void *event_data);
static void mqtt_publish_task(void* pvParameters);
static esp_err_t save_wifi_config_to_nvs(const char* key, const char* value, size_t length);
static void wifi_init(void);
static void mqtt_app_start(void);

// WiFi initialization function
static void wifi_init(void) {
    ESP_ERROR_CHECK(esp_netif_init());
    ESP_ERROR_CHECK(esp_event_loop_create_default());
    esp_netif_create_default_wifi_sta();

    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_wifi_init(&cfg));

    wifi_event_group = xEventGroupCreate();
    ESP_ERROR_CHECK(esp_event_handler_instance_register(WIFI_EVENT, ESP_EVENT_ANY_ID, &wifi_event_handler, NULL, &wifi_handler_event_instance));
    ESP_ERROR_CHECK(esp_event_handler_instance_register(IP_EVENT, IP_EVENT_STA_GOT_IP, &ip_event_handler, NULL, &got_ip_event_instance));

    // Read WiFi credentials from NVS
    nvs_handle_t nvs_handle;
    char ssid[33] = {0};
    char password[65] = {0};
    size_t ssid_len = sizeof(ssid);
    size_t pass_len = sizeof(password);

    esp_err_t err = nvs_open(NVS_NAMESPACE, NVS_READONLY, &nvs_handle);
    if (err == ESP_OK) {
        err = nvs_get_str(nvs_handle, NVS_KEY_SSID, ssid, &ssid_len);
        if (err == ESP_OK) {
            err = nvs_get_str(nvs_handle, NVS_KEY_PASS, password, &pass_len);
        }
        nvs_close(nvs_handle);
    }

    if (err == ESP_OK && ssid[0] != '\0' && password[0] != '\0') {
        wifi_config_t wifi_config = {
            .sta = {
                .threshold.authmode = WIFI_AUTH_WPA2_PSK,
                .pmf_cfg = {
                    .capable = true,
                    .required = false
                },
            },
        };
        memcpy(wifi_config.sta.ssid, ssid, ssid_len);
        memcpy(wifi_config.sta.password, password, pass_len);

        ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA));
        ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_STA, &wifi_config));
        ESP_ERROR_CHECK(esp_wifi_start());

        ESP_LOGI(GATTS_TAG, "wifi_init finished. Connecting to SSID:%s", ssid);
    } else {
        ESP_LOGI(GATTS_TAG, "No WiFi credentials found in NVS");
    }
}

// MQTT event handler
static void mqtt_event_handler(void *handler_args, esp_event_base_t base, int32_t event_id, void *event_data) {
    esp_mqtt_event_handle_t event = event_data;
    mqtt_client = event->client;

    switch (event_id) {
        case MQTT_EVENT_CONNECTED:
            mqtt_connected = true;
            ESP_LOGI(GATTS_TAG, "MQTT Connected");
            if (mqtt_publish_task_handle == NULL) {
                xTaskCreate(mqtt_publish_task, "mqtt_publish_task", 4096, NULL, 5, &mqtt_publish_task_handle);
            }
            break;
        case MQTT_EVENT_DISCONNECTED:
            mqtt_connected = false;
            ESP_LOGI(GATTS_TAG, "MQTT Disconnected");
            if (mqtt_publish_task_handle != NULL) {
                vTaskDelete(mqtt_publish_task_handle);
                mqtt_publish_task_handle = NULL;
            }
            break;
        case MQTT_EVENT_PUBLISHED:
            ESP_LOGI(GATTS_TAG, "MQTT Published, msg_id=%d", event->msg_id);
            break;
        default:
            break;
    }
}

// Add i2c_device_t as a global variable
static i2c_device_t i2c;

// Modify generate_sensor_value to handle real temperature readings
static float generate_sensor_value(int sensor_type) {
    static int32_t result;
    
    switch (sensor_type) {
        case 0: // Soil moisture (0-100%)
            return (float)(rand() % 100);
        case 1: // Temperature (from sensor)
            trigger_measurment(i2c);
            read_temp(i2c, &result);
            int32_t temp = bmp280_compensate_T_int32(result);
            return (float)temp / 100.0; // Convert to degrees Celsius
        case 2: // Water pump (0-1)
            return (float)(rand() % 2);
        case 3: // Light sensor (0-1000 lux)
            return (float)(rand() % 1000);
        case 4: // Light emitter (0-100%)
            return (float)(rand() % 100);
        default:
            return 0.0;
    }
}

// Modified MQTT publishing task with JSON data
static void mqtt_publish_task(void* pvParameters) {
    int counter = 0;
    char *json_string = NULL;
    struct timeval tv;
    time_t t;
    char timestamp[64];

    while (1) {
        if (mqtt_connected) {
            // Get current timestamp
            gettimeofday(&tv, NULL);
            t = tv.tv_sec;
            strftime(timestamp, sizeof(timestamp), "%Y-%m-%d %H:%M:%S", localtime(&t));

            // Create JSON object
            cJSON *root = cJSON_CreateObject();
            if (root == NULL) {
                ESP_LOGE(GATTS_TAG, "Failed to create JSON object");
                continue;
            }

            // Add timestamp to JSON
            cJSON_AddStringToObject(root, "timestamp", timestamp);

            // Add sensor value based on counter
            float sensor_value = generate_sensor_value(counter % 5);
            cJSON_AddNumberToObject(root, "value", sensor_value);

            // Convert JSON to string
            json_string = cJSON_Print(root);
            if (json_string != NULL) {
                // Publish to appropriate topic based on counter
                switch (counter % 5) {
                    case 0:
                        esp_mqtt_client_publish(mqtt_client, SOIL_MOISTURE_TOPIC_SUFFIX, 
                            json_string, strlen(json_string), 1, 0);
                        ESP_LOGI(GATTS_TAG, "Published soil moisture: %s", json_string);
                        break;
                    case 1:
                        esp_mqtt_client_publish(mqtt_client, TEMPERATURE_TOPIC_SUFFIX, 
                            json_string, strlen(json_string), 1, 0);
                        ESP_LOGI(GATTS_TAG, "Published temperature: %s", json_string);
                        break;
                    case 2:
                        esp_mqtt_client_publish(mqtt_client, WATER_PUMP_TOPIC_SUFFIX, 
                            json_string, strlen(json_string), 1, 0);
                        ESP_LOGI(GATTS_TAG, "Published water pump: %s", json_string);
                        break;
                    case 3:
                        esp_mqtt_client_publish(mqtt_client, LIGHT_SENSOR_TOPIC_SUFFIX, 
                            json_string, strlen(json_string), 1, 0);
                        ESP_LOGI(GATTS_TAG, "Published light sensor: %s", json_string);
                        break;
                    case 4:
                        esp_mqtt_client_publish(mqtt_client, LIGHT_EMITTER_TOPIC_SUFFIX, 
                            json_string, strlen(json_string), 1, 0);
                        ESP_LOGI(GATTS_TAG, "Published light emitter: %s", json_string);
                        break;
                }
                
                // Free JSON string
                free(json_string);
            }

            // Delete JSON object
            cJSON_Delete(root);
            counter++;
        }
        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}

// Start MQTT client
static void mqtt_app_start(void) {
    esp_mqtt_client_config_t mqtt_cfg = {
        .broker.address.uri = CONFIG_BROKER_URL,
    };
    mqtt_client = esp_mqtt_client_init(&mqtt_cfg);
    esp_mqtt_client_register_event(mqtt_client, ESP_EVENT_ANY_ID, mqtt_event_handler, NULL);
    esp_mqtt_client_start(mqtt_client);
}

// WiFi event handler
static void wifi_event_handler(void* arg, esp_event_base_t event_base, int32_t event_id, void* event_data) {
    if (event_base == WIFI_EVENT) {
        switch (event_id) {
            case WIFI_EVENT_STA_START:
                ESP_LOGI(GATTS_TAG, "Connecting to AP...");
                esp_wifi_connect();
                break;
            case WIFI_EVENT_STA_CONNECTED:
                wifi_connected = true;
                ESP_LOGI(GATTS_TAG, "Connected to AP");
                break;
            case WIFI_EVENT_STA_DISCONNECTED:
                wifi_connected = false;
                ESP_LOGI(GATTS_TAG, "Disconnected from AP");
                esp_wifi_connect();
                break;
            default:
                break;
        }
    }
}

// IP event handler
static void ip_event_handler(void* arg, esp_event_base_t event_base, int32_t event_id, void* event_data) {
    if (event_base == IP_EVENT && event_id == IP_EVENT_STA_GOT_IP) {
        ip_event_got_ip_t* event = (ip_event_got_ip_t*) event_data;
        ESP_LOGI(GATTS_TAG, "Got IP: " IPSTR, IP2STR(&event->ip_info.ip));
        xEventGroupSetBits(wifi_event_group, WIFI_SUCCESS);
    }
}

// Save WiFi configuration to NVS
static esp_err_t save_wifi_config_to_nvs(const char* key, const char* value, size_t length) {
    nvs_handle_t nvs_handle;
    esp_err_t err = nvs_open(NVS_NAMESPACE, NVS_READWRITE, &nvs_handle);
    if (err != ESP_OK) {
        return err;
    }
    err = nvs_set_str(nvs_handle, key, value);
    if (err == ESP_OK) {
        err = nvs_commit(nvs_handle);
    }
    nvs_close(nvs_handle);
    return err;
}

// GAP event handler
static void gap_event_handler(esp_gap_ble_cb_event_t event, esp_ble_gap_cb_param_t *param) {
    switch (event) {
        case ESP_GAP_BLE_ADV_DATA_SET_COMPLETE_EVT:
            adv_config_done &= (~adv_config_flag);
            if (adv_config_done == 0) {
                esp_ble_gap_start_advertising(&adv_params);
            }
            break;
        case ESP_GAP_BLE_SCAN_RSP_DATA_SET_COMPLETE_EVT:
            adv_config_done &= (~scan_rsp_config_flag);
            if (adv_config_done == 0) {
                esp_ble_gap_start_advertising(&adv_params);
            }
            break;
        case ESP_GAP_BLE_ADV_START_COMPLETE_EVT:
            if (param->adv_start_cmpl.status != ESP_BT_STATUS_SUCCESS) {
                ESP_LOGE(GATTS_TAG, "Advertising start failed");
            }
            break;
        default:
            break;
    }
}

// GATTS event handler
static void gatts_event_handler(esp_gatts_cb_event_t event, esp_gatt_if_t gatts_if, esp_ble_gatts_cb_param_t *param) {
    if (event == ESP_GATTS_REG_EVT) {
        if (param->reg.status == ESP_GATT_OK) {
            gl_profile_tab[param->reg.app_id].gatts_if = gatts_if;
        } else {
            ESP_LOGE(GATTS_TAG, "Register app failed, app_id %04x, status %d",
                    param->reg.app_id,
                    param->reg.status);
            return;
        }
    }

    for (int idx = 0; idx < PROFILE_NUM; idx++) {
        if (gatts_if == ESP_GATT_IF_NONE || gatts_if == gl_profile_tab[idx].gatts_if) {
            if (gl_profile_tab[idx].gatts_cb) {
                gl_profile_tab[idx].gatts_cb(event, gatts_if, param);
            }
        }
    }
}

// WiFi profile event handler
static void gatts_profile_wifi_event_handler(esp_gatts_cb_event_t event, esp_gatt_if_t gatts_if, esp_ble_gatts_cb_param_t *param) {
    switch (event) {
        case ESP_GATTS_REG_EVT: {
            esp_err_t set_dev_name_ret = esp_ble_gap_set_device_name("ESP_WIFI_CONFIG");
            if (set_dev_name_ret) {
                ESP_LOGE(GATTS_TAG, "set device name failed, error code = %x", set_dev_name_ret);
            }
            esp_err_t ret = esp_ble_gap_config_adv_data(&adv_data);
            if (ret) {
                ESP_LOGE(GATTS_TAG, "config adv data failed, error code = %x", ret);
            }
            adv_config_done |= adv_config_flag;
            ret = esp_ble_gap_config_adv_data(&scan_rsp_data);
            if (ret) {
                ESP_LOGE(GATTS_TAG, "config scan response data failed, error code = %x", ret);
            }
            adv_config_done |= scan_rsp_config_flag;
            break;
        }
        case ESP_GATTS_WRITE_EVT: {
            if (param->write.handle == gl_profile_tab[PROFILE_WIFI_APP_ID].ssid_char_handle) {
                ESP_LOGI(GATTS_TAG, "SSID Write, len %d", param->write.len);
                save_wifi_config_to_nvs(NVS_KEY_SSID, (char*)param->write.value, param->write.len);
            } else if (param->write.handle == gl_profile_tab[PROFILE_WIFI_APP_ID].pass_char_handle) {
                ESP_LOGI(GATTS_TAG, "Password Write, len %d", param->write.len);
                save_wifi_config_to_nvs(NVS_KEY_PASS, (char*)param->write.value, param->write.len);
            }
            esp_ble_gatts_send_response(gatts_if, param->write.conn_id, param->write.trans_id, ESP_GATT_OK, NULL);
            break;
        }
        default:
            break;
    }
}

void app_main(void) {
    // Initialize NVS
    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        ESP_ERROR_CHECK(nvs_flash_erase());
        ret = nvs_flash_init();
    }
    ESP_ERROR_CHECK(ret);

    // Initialize BLE
    ESP_ERROR_CHECK(esp_bt_controller_mem_release(ESP_BT_MODE_CLASSIC_BT));
    esp_bt_controller_config_t bt_cfg = BT_CONTROLLER_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_bt_controller_init(&bt_cfg));
    ESP_ERROR_CHECK(esp_bt_controller_enable(ESP_BT_MODE_BLE));
    ESP_ERROR_CHECK(esp_bluedroid_init());
    ESP_ERROR_CHECK(esp_bluedroid_enable());

    // Register BLE callbacks
    ESP_ERROR_CHECK(esp_ble_gatts_register_callback(gatts_event_handler));
    ESP_ERROR_CHECK(esp_ble_gap_register_callback(gap_event_handler));
    ESP_ERROR_CHECK(esp_ble_gatts_app_register(PROFILE_WIFI_APP_ID));
    ESP_ERROR_CHECK(esp_ble_gatt_set_local_mtu(500));

    // Initialize WiFi
    wifi_init();

    // Initialize I2C (moved before the main loop)
    i2c_init(&i2c, I2C_SDA_PIN, I2C_SCL_PIN, "I2C_TEST");
    get_calibration_params(i2c);

    // Main loop
    while (1) {
        if (wifi_connected && !mqtt_connected) {
            mqtt_app_start();
        }
        vTaskDelay(1000 / portTICK_PERIOD_MS);
    }
}

// void app_main() {
//     i2c_device_t i2c;
//     i2c_init(&i2c, I2C_SDA_PIN, I2C_SCL_PIN, "I2C_TEST");
    
//     int32_t result;
//     get_calibration_params(i2c);
//     trigger_measurment(i2c);
//     read_temp(i2c, &result);
//     printf("Raw temperature: %ld\n", result);
//     int32_t temp = bmp280_compensate_T_int32(result);
//     printf("Compensated temperature: %ld\n", temp);
// }
