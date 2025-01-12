#pragma once

#define WIFI_CONFIG_SERVICE_UUID_128 "WIFI_CONFIG"
#define WIFI_SSID_CHAR_UUID_128    "SET_SSID"
#define WIFI_PASS_CHAR_UUID_128    "SET_PASSWORD"
#define WIFI_MQTT_URL_CHAR_UUID_128 "SET_MQTT_URL"
#define GATTS_NUM_HANDLE_WIFI     13

#define PROFILE_NUM 1
#define PROFILE_WIFI_APP_ID 0

#define PREPARE_BUF_MAX_SIZE 1024
#define adv_config_flag      (1 << 0)
#define scan_rsp_config_flag (1 << 1)

#define GATTS_TAG "GATTS"

void iot_ble_init(void);
void iot_ble_start(void);
void iot_ble_stop(void);