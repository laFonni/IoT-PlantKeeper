#include <string.h>
#include <esp_err.h>
#include <esp_log.h>
#include <esp_bt.h>
#include "esp_gap_ble_api.h"
#include "esp_gatts_api.h"
#include "esp_bt_defs.h"
#include "esp_bt_main.h"
#include "esp_gatt_common_api.h"

#include "iot_ble.h"
#include "iot_nvs.h"

static void gap_event_handler(esp_gap_ble_cb_event_t event, esp_ble_gap_cb_param_t *param);
static void gatts_profile_wifi_event_handler(esp_gatts_cb_event_t event, esp_gatt_if_t gatts_if, esp_ble_gatts_cb_param_t *param);
static void gatts_event_handler(esp_gatts_cb_event_t event, esp_gatt_if_t gatts_if, esp_ble_gatts_cb_param_t *param);

// static uint8_t char1_str[] = {0x11,0x22,0x33};
static esp_gatt_char_prop_t a_property = 0;

// static esp_attr_value_t gatts_demo_char1_val =
// {
//     .attr_max_len = GATTS_DEMO_CHAR_VAL_LEN_MAX,
//     .attr_len     = sizeof(char1_str),
//     .attr_value   = char1_str,
// };

static uint8_t adv_config_done = 0;

static uint8_t adv_service_uuid128[32] = {
    /* LSB <--------------------------------------------------------------------------------> MSB */
    //first uuid, 16bit, [12],[13] is the value
    0xfb, 0x34, 0x9b, 0x5f, 0x80, 0x00, 0x00, 0x80, 0x00, 0x10, 0x00, 0x00, 0xEE, 0x00, 0x00, 0x00,
    //second uuid, 32bit, [12], [13], [14], [15] is the value
    0xfb, 0x34, 0x9b, 0x5f, 0x80, 0x00, 0x00, 0x80, 0x00, 0x10, 0x00, 0x00, 0xFF, 0x00, 0x00, 0x00,
};

// The length of adv data must be less than 31 bytes
//static uint8_t test_manufacturer[TEST_MANUFACTURER_DATA_LEN] =  {0x12, 0x23, 0x45, 0x56};
//adv data
static esp_ble_adv_data_t adv_data = {
    .set_scan_rsp = false,
    .include_name = true,
    .include_txpower = false,
    .min_interval = 0x0006, //slave connection min interval, Time = min_interval * 1.25 msec
    .max_interval = 0x0010, //slave connection max interval, Time = max_interval * 1.25 msec
    .appearance = 0x00,
    .manufacturer_len = 0, //TEST_MANUFACTURER_DATA_LEN,
    .p_manufacturer_data =  NULL, //&test_manufacturer[0],
    .service_data_len = 0,
    .p_service_data = NULL,
    .service_uuid_len = sizeof(adv_service_uuid128),
    .p_service_uuid = adv_service_uuid128,
    .flag = (ESP_BLE_ADV_FLAG_GEN_DISC | ESP_BLE_ADV_FLAG_BREDR_NOT_SPT),
};
// scan response data
static esp_ble_adv_data_t scan_rsp_data = {
    .set_scan_rsp = true,
    .include_name = true,
    .include_txpower = true,
    //.min_interval = 0x0006,
    //.max_interval = 0x0010,
    .appearance = 0x00,
    .manufacturer_len = 0, //TEST_MANUFACTURER_DATA_LEN,
    .p_manufacturer_data =  NULL, //&test_manufacturer[0],
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
    //.peer_addr            =
    //.peer_addr_type       =
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
    uint16_t mqtt_url_char_handle;
    uint16_t char_handle;
    esp_bt_uuid_t char_uuid;
    esp_gatt_perm_t perm;
    esp_gatt_char_prop_t property;
    uint16_t descr_handle;
    esp_bt_uuid_t descr_uuid;
};

/* One gatt-based profile one app_id and one gatts_if, this array will store the gatts_if returned by ESP_GATTS_REG_EVT */
static struct gatts_profile_inst gl_profile_tab[PROFILE_NUM] = {
    [PROFILE_WIFI_APP_ID] = {
        .gatts_cb = gatts_profile_wifi_event_handler,
        .gatts_if = ESP_GATT_IF_NONE,
    },
};

typedef struct {
    uint8_t                 *prepare_buf;
    int                     prepare_len;
} prepare_type_env_t;

static prepare_type_env_t a_prepare_write_env;

static void example_write_event_env(esp_gatt_if_t gatts_if, prepare_type_env_t *prepare_write_env, esp_ble_gatts_cb_param_t *param);
static void example_exec_write_event_env(prepare_type_env_t *prepare_write_env, esp_ble_gatts_cb_param_t *param);

void iot_ble_init(void) {
    esp_err_t ret;
    ret = esp_bt_controller_mem_release(ESP_BT_MODE_CLASSIC_BT);
    if (ret) {
        printf("%s mem release failed: %s\n", __func__, esp_err_to_name(ret));
        return;
    }
}

void iot_ble_start(void) {
    esp_err_t ret;
    // ESP_ERROR_CHECK(esp_bt_controller_mem_release(ESP_BT_MODE_CLASSIC_BT));

    esp_bt_controller_config_t bt_cfg = BT_CONTROLLER_INIT_CONFIG_DEFAULT();
    ret = esp_bt_controller_init(&bt_cfg);
    if (ret) {
        ESP_LOGE(GATTS_TAG, "%s initialize controller failed: %s", __func__, esp_err_to_name(ret));
        return;
    }

    ret = esp_bt_controller_enable(ESP_BT_MODE_BLE);
    if (ret) {
        ESP_LOGE(GATTS_TAG, "%s enable controller failed: %s", __func__, esp_err_to_name(ret));
        return;
    }

    ret = esp_bluedroid_init();
    if (ret) {
        ESP_LOGE(GATTS_TAG, "%s init bluetooth failed: %s", __func__, esp_err_to_name(ret));
        return;
    }
    ret = esp_bluedroid_enable();
    if (ret) {
        ESP_LOGE(GATTS_TAG, "%s enable bluetooth failed: %s", __func__, esp_err_to_name(ret));
        return;
    }

    ret = esp_ble_gatts_register_callback(gatts_event_handler);
    if (ret){
        ESP_LOGE(GATTS_TAG, "gatts register error, error code = %x", ret);
        return;
    }
    ret = esp_ble_gap_register_callback(gap_event_handler);
    if (ret){
        ESP_LOGE(GATTS_TAG, "gap register error, error code = %x", ret);
        return;
    }
    ret = esp_ble_gatts_app_register(PROFILE_WIFI_APP_ID);
    if (ret){
        ESP_LOGE(GATTS_TAG, "gatts app register error, error code = %x", ret);
        return;
    }
    esp_err_t local_mtu_ret = esp_ble_gatt_set_local_mtu(500);
    if (local_mtu_ret){
        ESP_LOGE(GATTS_TAG, "set local  MTU failed, error code = %x", local_mtu_ret);
    }
}

void iot_ble_stop(void) {
    esp_err_t err;
    err = esp_bluedroid_disable();
    if (err) {
        ESP_LOGE(GATTS_TAG, "%s disable bluedroid failed: %s", __func__, esp_err_to_name(err));
        return;
    }
    err = esp_bluedroid_deinit();
    if (err) {
        ESP_LOGE(GATTS_TAG, "%s deinitialize bluedroid failed: %s", __func__, esp_err_to_name(err));
        return;
    }
    err = esp_bt_controller_disable();
    if (err) {
        ESP_LOGE(GATTS_TAG, "%s disable controller failed: %s", __func__, esp_err_to_name(err));
        return;
    }
    err = esp_bt_controller_deinit();
    if (err) {
        ESP_LOGE(GATTS_TAG, "%s deinitialize controller failed: %s", __func__, esp_err_to_name(err));
        return;
    }
}

static void gap_event_handler(esp_gap_ble_cb_event_t event, esp_ble_gap_cb_param_t *param)
{
    switch (event) {
    case ESP_GAP_BLE_ADV_DATA_SET_COMPLETE_EVT:
        adv_config_done &= (~adv_config_flag);
        if (adv_config_done == 0){
            esp_ble_gap_start_advertising(&adv_params);
        }
        break;
    case ESP_GAP_BLE_SCAN_RSP_DATA_SET_COMPLETE_EVT:
        adv_config_done &= (~scan_rsp_config_flag);
        if (adv_config_done == 0){
            esp_ble_gap_start_advertising(&adv_params);
        }
        break;
    case ESP_GAP_BLE_ADV_START_COMPLETE_EVT:
        //advertising start complete event to indicate advertising start successfully or failed
        if (param->adv_start_cmpl.status != ESP_BT_STATUS_SUCCESS) {
            ESP_LOGE(GATTS_TAG, "Advertising start failed, status %d", param->adv_start_cmpl.status);
            break;
        }
        ESP_LOGI(GATTS_TAG, "Advertising start successfully");
        break;
    case ESP_GAP_BLE_ADV_STOP_COMPLETE_EVT:
        if (param->adv_start_cmpl.status != ESP_BT_STATUS_SUCCESS) {
            ESP_LOGE(GATTS_TAG, "Advertising stop failed, status %d", param->adv_stop_cmpl.status);
            break;
        }
        ESP_LOGI(GATTS_TAG, "Advertising stop successfully");
        break;
    case ESP_GAP_BLE_UPDATE_CONN_PARAMS_EVT:
         ESP_LOGI(GATTS_TAG, "Connection params update, status %d, conn_int %d, latency %d, timeout %d",
                  param->update_conn_params.status,
                  param->update_conn_params.conn_int,
                  param->update_conn_params.latency,
                  param->update_conn_params.timeout);
        break;
    case ESP_GAP_BLE_SET_PKT_LENGTH_COMPLETE_EVT:
        ESP_LOGI(GATTS_TAG, "Packet length update, status %d, rx %d, tx %d",
                  param->pkt_data_length_cmpl.status,
                  param->pkt_data_length_cmpl.params.rx_len,
                  param->pkt_data_length_cmpl.params.tx_len);
        break;
    default:
        break;
    }
}

static void gatts_profile_wifi_event_handler(esp_gatts_cb_event_t event, esp_gatt_if_t gatts_if, esp_ble_gatts_cb_param_t *param) {
    switch (event) {
    case ESP_GATTS_REG_EVT: {
        ESP_LOGI(GATTS_TAG, "REGISTER_APP_EVT, status %d, app_id %d\n", param->reg.status, param->reg.app_id);
        gl_profile_tab[PROFILE_WIFI_APP_ID].service_id.is_primary = true;
        gl_profile_tab[PROFILE_WIFI_APP_ID].service_id.id.inst_id = 0x00;
        // gl_profile_tab[PROFILE_WIFI_APP_ID].service_id.id.uuid.len = ESP_UUID_LEN_128;
        // memcpy(gl_profile_tab[PROFILE_WIFI_APP_ID].service_id.id.uuid.uuid.uuid128, 
        //        WIFI_CONFIG_SERVICE_UUID_128, ESP_UUID_LEN_128);
        gl_profile_tab[PROFILE_WIFI_APP_ID].service_id.id.uuid.len = ESP_UUID_LEN_16;
        gl_profile_tab[PROFILE_WIFI_APP_ID].service_id.id.uuid.uuid.uuid16 = WIFI_CONFIG_SERVICE_UUID_16;

        esp_err_t set_dev_name_ret = esp_ble_gap_set_device_name("ESP_WIFI_CONFIG");
        if (set_dev_name_ret){
            ESP_LOGE(GATTS_TAG, "set device name failed, error code = %x", set_dev_name_ret);
        }
        //config adv data
        esp_err_t ret = esp_ble_gap_config_adv_data(&adv_data);
        if (ret){
            ESP_LOGE(GATTS_TAG, "config adv data failed, error code = %x", ret);
        }
        adv_config_done |= adv_config_flag;
        //config scan response data
        ret = esp_ble_gap_config_adv_data(&scan_rsp_data);
        if (ret){
            ESP_LOGE(GATTS_TAG, "config scan response data failed, error code = %x", ret);
        }
        adv_config_done |= scan_rsp_config_flag;

        esp_ble_gatts_create_service(gatts_if, &gl_profile_tab[PROFILE_WIFI_APP_ID].service_id, GATTS_NUM_HANDLE_WIFI);
        break;
    }
    case ESP_GATTS_READ_EVT: {
        ESP_LOGI(GATTS_TAG, "Characteristic read, conn_id %d, trans_id %" PRIu32 ", handle %d", param->read.conn_id, param->read.trans_id, param->read.handle);
        esp_gatt_rsp_t rsp;
        memset(&rsp, 0, sizeof(esp_gatt_rsp_t));
        rsp.attr_value.handle = param->read.handle;
        rsp.attr_value.len = 4;
        rsp.attr_value.value[0] = 0xde;
        rsp.attr_value.value[1] = 0xed;
        rsp.attr_value.value[2] = 0xbe;
        rsp.attr_value.value[3] = 0xef;
        esp_ble_gatts_send_response(gatts_if, param->read.conn_id, param->read.trans_id,
                                    ESP_GATT_OK, &rsp);
        break;
    }
    case ESP_GATTS_WRITE_EVT: {
        ESP_LOGI(GATTS_TAG, "Characteristic write, conn_id %d, trans_id %" PRIu32 ", handle %d", 
                 param->write.conn_id, param->write.trans_id, param->write.handle);
        
        if (!param->write.is_prep) {
            // Create a null-terminated string from the received data
            char* str_value = (char*)malloc(param->write.len + 1);
            if (str_value) {
                memcpy(str_value, param->write.value, param->write.len);
                str_value[param->write.len] = '\0';

                printf("%d\n", param->write.handle);
                printf("%d\n", gl_profile_tab[PROFILE_WIFI_APP_ID].ssid_char_handle);
                
                // Identify which characteristic was written to and save to NVS
                if (param->write.handle == gl_profile_tab[PROFILE_WIFI_APP_ID].ssid_char_handle) {
                    ESP_LOGI(GATTS_TAG, "SSID received: %s", str_value);
                    save_wifi_config_to_nvs(NVS_KEY_SSID, str_value, param->write.len);
                } else if (param->write.handle == gl_profile_tab[PROFILE_WIFI_APP_ID].pass_char_handle) {
                    ESP_LOGI(GATTS_TAG, "Password received: %s", str_value);
                    save_wifi_config_to_nvs(NVS_KEY_PASS, str_value, param->write.len);
                } else if (param->write.handle == gl_profile_tab[PROFILE_WIFI_APP_ID].mqtt_url_char_handle) {
                    ESP_LOGI(GATTS_TAG, "MQTT URL received: %s", str_value);
                    save_wifi_config_to_nvs(NVS_KEY_MQTT_URL, str_value, param->write.len);
                } else {
                    ESP_LOGI(GATTS_TAG, "Unknown characteristic value: %s", str_value);
                    ESP_LOG_BUFFER_HEX(GATTS_TAG, param->write.value, param->write.len);
                }
                
                free(str_value);
            }

            // Keep existing descriptor handling code
            if (gl_profile_tab[PROFILE_WIFI_APP_ID].descr_handle == param->write.handle && param->write.len == 2) {
                uint16_t descr_value = param->write.value[1]<<8 | param->write.value[0];
                if (descr_value == 0x0001){
                    if (a_property & ESP_GATT_CHAR_PROP_BIT_NOTIFY){
                        ESP_LOGI(GATTS_TAG, "Notification enable");
                        uint8_t notify_data[15];
                        for (int i = 0; i < sizeof(notify_data); ++i)
                        {
                            notify_data[i] = i%0xff;
                        }
                        //the size of notify_data[] need less than MTU size
                        esp_ble_gatts_send_indicate(gatts_if, param->write.conn_id, gl_profile_tab[PROFILE_WIFI_APP_ID].char_handle,
                                                sizeof(notify_data), notify_data, false);
                    }
                }else if (descr_value == 0x0002){
                    if (a_property & ESP_GATT_CHAR_PROP_BIT_INDICATE){
                        ESP_LOGI(GATTS_TAG, "Indication enable");
                        uint8_t indicate_data[15];
                        for (int i = 0; i < sizeof(indicate_data); ++i)
                        {
                            indicate_data[i] = i%0xff;
                        }
                        //the size of indicate_data[] need less than MTU size
                        esp_ble_gatts_send_indicate(gatts_if, param->write.conn_id, gl_profile_tab[PROFILE_WIFI_APP_ID].char_handle,
                                                sizeof(indicate_data), indicate_data, true);
                    }
                }
                else if (descr_value == 0x0000){
                    ESP_LOGI(GATTS_TAG, "Notification/Indication disable");
                }else{
                    ESP_LOGE(GATTS_TAG, "Unknown descriptor value");
                    ESP_LOG_BUFFER_HEX(GATTS_TAG, param->write.value, param->write.len);
                }

            }
        }
        example_write_event_env(gatts_if, &a_prepare_write_env, param);
        break;
    }
    case ESP_GATTS_EXEC_WRITE_EVT:
        ESP_LOGI(GATTS_TAG,"Execute write");
        esp_ble_gatts_send_response(gatts_if, param->write.conn_id, param->write.trans_id, ESP_GATT_OK, NULL);
        example_exec_write_event_env(&a_prepare_write_env, param);
        break;
    case ESP_GATTS_MTU_EVT:
        ESP_LOGI(GATTS_TAG, "MTU exchange, MTU %d", param->mtu.mtu);
        break;
    case ESP_GATTS_UNREG_EVT:
        break;
    case ESP_GATTS_CREATE_EVT:
        ESP_LOGI(GATTS_TAG, "CREATE_SERVICE_EVT, status %d, service_handle %d\n", param->create.status, param->create.service_handle);
        gl_profile_tab[PROFILE_WIFI_APP_ID].service_handle = param->create.service_handle;

        // Add SSID characteristic with 128-bit UUID
        // esp_bt_uuid_t ssid_uuid = {
        //     .len = ESP_UUID_LEN_128,
        //     .uuid.uuid128 = WIFI_SSID_CHAR_UUID_128,
        // };
        esp_bt_uuid_t ssid_uuid = {
            .len = ESP_UUID_LEN_16,
            .uuid.uuid16 = WIFI_SSID_CHAR_UUID_16,
        };
        esp_ble_gatts_add_char(gl_profile_tab[PROFILE_WIFI_APP_ID].service_handle,
                              &ssid_uuid,
                              ESP_GATT_PERM_WRITE,
                              ESP_GATT_CHAR_PROP_BIT_WRITE,
                              NULL, NULL);
                              
        // Add Password characteristic with 128-bit UUID
        // esp_bt_uuid_t pass_uuid = {
        //     .len = ESP_UUID_LEN_128,
        //     .uuid.uuid128 = WIFI_PASS_CHAR_UUID_128,
        // };
        esp_bt_uuid_t pass_uuid = {
            .len = ESP_UUID_LEN_16,
            .uuid.uuid16 = WIFI_PASS_CHAR_UUID_16,
        };
        esp_ble_gatts_add_char(gl_profile_tab[PROFILE_WIFI_APP_ID].service_handle,
                              &pass_uuid,
                              ESP_GATT_PERM_WRITE,
                              ESP_GATT_CHAR_PROP_BIT_WRITE,
                              NULL, NULL);

        // Add MQTT URL characteristic with 128-bit UUID
        // esp_bt_uuid_t mqtt_url_uuid = {
        //     .len = ESP_UUID_LEN_128,
        //     .uuid.uuid128 = WIFI_MQTT_URL_CHAR_UUID_128,
        // };
        esp_bt_uuid_t mqtt_url_uuid = {
            .len = ESP_UUID_LEN_16,
            .uuid.uuid16 = WIFI_MQTT_URL_CHAR_UUID_16,
        };
        esp_ble_gatts_add_char(gl_profile_tab[PROFILE_WIFI_APP_ID].service_handle,
                              &mqtt_url_uuid,
                              ESP_GATT_PERM_WRITE,
                              ESP_GATT_CHAR_PROP_BIT_WRITE,
                              NULL, NULL);

        esp_ble_gatts_start_service(gl_profile_tab[PROFILE_WIFI_APP_ID].service_handle);
        break;
    case ESP_GATTS_ADD_INCL_SRVC_EVT:
        break;
    case ESP_GATTS_ADD_CHAR_EVT: {
        uint16_t length = 0;
        const uint8_t *prf_char;

        ESP_LOGI(GATTS_TAG, "Characteristic add, status %d, attr_handle %d, service_handle %d",
                param->add_char.status, param->add_char.attr_handle, param->add_char.service_handle);
        
        // Store the handle based on the UUID
        // if (strcmp((char*)(param->add_char.char_uuid.uuid.uuid128), WIFI_SSID_CHAR_UUID_128) == 0) {
        //     gl_profile_tab[PROFILE_WIFI_APP_ID].ssid_char_handle = param->add_char.attr_handle;
        // } else if (strcmp((char*)(param->add_char.char_uuid.uuid.uuid128), WIFI_PASS_CHAR_UUID_128) == 0) {
        //     gl_profile_tab[PROFILE_WIFI_APP_ID].pass_char_handle = param->add_char.attr_handle;
        // } else if (strcmp((char*)(param->add_char.char_uuid.uuid.uuid128), WIFI_MQTT_URL_CHAR_UUID_128) == 0) {
        //     gl_profile_tab[PROFILE_WIFI_APP_ID].mqtt_url_char_handle = param->add_char.attr_handle;
        // }
        if (param->add_char.char_uuid.uuid.uuid16 == WIFI_SSID_CHAR_UUID_16) {
            gl_profile_tab[PROFILE_WIFI_APP_ID].ssid_char_handle = param->add_char.attr_handle;
        } else if (param->add_char.char_uuid.uuid.uuid16 == WIFI_PASS_CHAR_UUID_16) {
            gl_profile_tab[PROFILE_WIFI_APP_ID].pass_char_handle = param->add_char.attr_handle;
        } else if (param->add_char.char_uuid.uuid.uuid16 == WIFI_MQTT_URL_CHAR_UUID_16) {
            gl_profile_tab[PROFILE_WIFI_APP_ID].mqtt_url_char_handle = param->add_char.attr_handle;
        }
        // Keep the rest of the existing code in this case...
        gl_profile_tab[PROFILE_WIFI_APP_ID].char_handle = param->add_char.attr_handle;
        gl_profile_tab[PROFILE_WIFI_APP_ID].descr_uuid.len = ESP_UUID_LEN_16;
        gl_profile_tab[PROFILE_WIFI_APP_ID].descr_uuid.uuid.uuid16 = ESP_GATT_UUID_CHAR_CLIENT_CONFIG;
        esp_err_t get_attr_ret = esp_ble_gatts_get_attr_value(param->add_char.attr_handle,  &length, &prf_char);
        if (get_attr_ret == ESP_FAIL){
            ESP_LOGE(GATTS_TAG, "ILLEGAL HANDLE");
        }

        ESP_LOGI(GATTS_TAG, "the gatts demo char length = %x", length);
        for(int i = 0; i < length; i++){
            ESP_LOGI(GATTS_TAG, "prf_char[%x] =%x",i,prf_char[i]);
        }
        esp_err_t add_descr_ret = esp_ble_gatts_add_char_descr(gl_profile_tab[PROFILE_WIFI_APP_ID].service_handle, &gl_profile_tab[PROFILE_WIFI_APP_ID].descr_uuid,
                                                                ESP_GATT_PERM_READ | ESP_GATT_PERM_WRITE, NULL, NULL);
        if (add_descr_ret){
            ESP_LOGE(GATTS_TAG, "add char descr failed, error code =%x", add_descr_ret);
        }

        // Add descriptors for the characteristics
        // if (strcmp((char*)(param->add_char.char_uuid.uuid.uuid128), WIFI_SSID_CHAR_UUID_128) == 0) {
        //     printf("SSID added descriptor\n");
        //     // SSID characteristic
        //     esp_bt_uuid_t descr_uuid = {
        //         .len = ESP_UUID_LEN_16,
        //         .uuid.uuid16 = ESP_GATT_UUID_CHAR_DESCRIPTION
        //     };
        //     esp_attr_value_t attribute_value = {
        //         .attr_max_len = strlen("SSID"),
        //         .attr_len = strlen("SSID"),
        //         .attr_value = (uint8_t *) "SSID"
        //     };
        //     esp_ble_gatts_add_char_descr(param->add_char.service_handle,
        //                                 &descr_uuid,
        //                                 ESP_GATT_PERM_READ,
        //                                 &attribute_value,
        //                                 ESP_GATT_AUTO_RSP);
        // } else if (param->add_char.char_uuid.uuid.uuid128[12] == 0xFF &&
        //            param->add_char.char_uuid.uuid.uuid128[13] == 0x02) {
        //     // Password characteristic
        //     esp_bt_uuid_t descr_uuid = {
        //         .len = ESP_UUID_LEN_16,
        //         .uuid.uuid16 = ESP_GATT_UUID_CHAR_DESCRIPTION
        //     };
        //     esp_ble_gatts_add_char_descr(param->add_char.service_handle,
        //                                 &descr_uuid,
        //                                 ESP_GATT_PERM_READ,
        //                                 NULL,
        //                                 (uint8_t*)"WiFi Password");
        // } else if (param->add_char.char_uuid.uuid.uuid128[12] == 0xFF &&
        //            param->add_char.char_uuid.uuid.uuid128[13] == 0x03) {
        //     // MQTT URL characteristic
        //     esp_bt_uuid_t descr_uuid = {
        //         .len = ESP_UUID_LEN_16,
        //         .uuid.uuid16 = ESP_GATT_UUID_CHAR_DESCRIPTION
        //     };

        //     esp_attr_value_t attribute_value = {
        //         .attr_max_len = strlen("MQTT URL"),
        //         .attr_len = strlen("MQTT URL"),
        //         .attr_value = (uint8_t *) "MQTT URL"
        //     };

        //     esp_ble_gatts_add_char_descr(param->add_char.service_handle,
        //                                 &descr_uuid,
        //                                 ESP_GATT_PERM_READ,
        //                                 &attribute_value,
        //                                 NULL);
        //     // esp_ble_gatts_add_char_descr(param->add_char.service_handle,
        //     //                             &descr_uuid,
        //     //                             ESP_GATT_PERM_READ,
        //     //                             NULL,
        //     //                             (uint8_t*)"MQTT URL");
        // }
        break;
    }
    case ESP_GATTS_ADD_CHAR_DESCR_EVT:
        gl_profile_tab[PROFILE_WIFI_APP_ID].descr_handle = param->add_char_descr.attr_handle;
        ESP_LOGI(GATTS_TAG, "Descriptor add, status %d, attr_handle %d, service_handle %d",
                 param->add_char_descr.status, param->add_char_descr.attr_handle, param->add_char_descr.service_handle);
        break;
    case ESP_GATTS_DELETE_EVT:
        break;
    case ESP_GATTS_START_EVT:
        ESP_LOGI(GATTS_TAG, "Service start, status %d, service_handle %d",
                 param->start.status, param->start.service_handle);
        break;
    case ESP_GATTS_STOP_EVT:
        break;
    case ESP_GATTS_CONNECT_EVT: {
        esp_ble_conn_update_params_t conn_params = {0};
        memcpy(conn_params.bda, param->connect.remote_bda, sizeof(esp_bd_addr_t));
        /* For the IOS system, please reference the apple official documents about the ble connection parameters restrictions. */
        conn_params.latency = 0;
        conn_params.max_int = 0x20;    // max_int = 0x20*1.25ms = 40ms
        conn_params.min_int = 0x10;    // min_int = 0x10*1.25ms = 20ms
        conn_params.timeout = 400;    // timeout = 400*10ms = 4000ms
        ESP_LOGI(GATTS_TAG, "Connected, conn_id %u, remote "ESP_BD_ADDR_STR"",
                 param->connect.conn_id, ESP_BD_ADDR_HEX(param->connect.remote_bda));
        gl_profile_tab[PROFILE_WIFI_APP_ID].conn_id = param->connect.conn_id;
        //start sent the update connection parameters to the peer device.
        esp_ble_gap_update_conn_params(&conn_params);
        break;
    }
    case ESP_GATTS_DISCONNECT_EVT:
        ESP_LOGI(GATTS_TAG, "Disconnected, remote "ESP_BD_ADDR_STR", reason 0x%02x",
                 ESP_BD_ADDR_HEX(param->disconnect.remote_bda), param->disconnect.reason);
        esp_ble_gap_start_advertising(&adv_params);
        break;
    case ESP_GATTS_CONF_EVT:
        ESP_LOGI(GATTS_TAG, "Confirm receive, status %d, attr_handle %d", param->conf.status, param->conf.handle);
        if (param->conf.status != ESP_GATT_OK){
            ESP_LOG_BUFFER_HEX(GATTS_TAG, param->conf.value, param->conf.len);
        }
        break;
    default:
        break;
    }
}

static void gatts_event_handler(esp_gatts_cb_event_t event, esp_gatt_if_t gatts_if, esp_ble_gatts_cb_param_t *param)
{
    /* If event is register event, store the gatts_if for each profile */
    if (event == ESP_GATTS_REG_EVT) {
        if (param->reg.status == ESP_GATT_OK) {
            gl_profile_tab[param->reg.app_id].gatts_if = gatts_if;
        } else {
            ESP_LOGI(GATTS_TAG, "Reg app failed, app_id %04x, status %d",
                    param->reg.app_id,
                    param->reg.status);
            return;
        }
    }

    // --------------------
    if (gatts_if == ESP_GATT_IF_NONE || gatts_if == gl_profile_tab[0].gatts_if) {
        if (gl_profile_tab[0].gatts_cb) {
            gl_profile_tab[0].gatts_cb(event, gatts_if, param);
        }
    }
    // --------------------
}

static void example_write_event_env(esp_gatt_if_t gatts_if, prepare_type_env_t *prepare_write_env, esp_ble_gatts_cb_param_t *param){
    esp_gatt_status_t status = ESP_GATT_OK;
    if (param->write.need_rsp){
        if (param->write.is_prep) {
            if (param->write.offset > PREPARE_BUF_MAX_SIZE) {
                status = ESP_GATT_INVALID_OFFSET;
            } else if ((param->write.offset + param->write.len) > PREPARE_BUF_MAX_SIZE) {
                status = ESP_GATT_INVALID_ATTR_LEN;
            }
            if (status == ESP_GATT_OK && prepare_write_env->prepare_buf == NULL) {
                prepare_write_env->prepare_buf = (uint8_t *)malloc(PREPARE_BUF_MAX_SIZE*sizeof(uint8_t));
                prepare_write_env->prepare_len = 0;
                if (prepare_write_env->prepare_buf == NULL) {
                    ESP_LOGE(GATTS_TAG, "Gatt_server prep no mem");
                    status = ESP_GATT_NO_RESOURCES;
                }
            }

            esp_gatt_rsp_t *gatt_rsp = (esp_gatt_rsp_t *)malloc(sizeof(esp_gatt_rsp_t));
            if (gatt_rsp) {
                gatt_rsp->attr_value.len = param->write.len;
                gatt_rsp->attr_value.handle = param->write.handle;
                gatt_rsp->attr_value.offset = param->write.offset;
                gatt_rsp->attr_value.auth_req = ESP_GATT_AUTH_REQ_NONE;
                memcpy(gatt_rsp->attr_value.value, param->write.value, param->write.len);
                esp_err_t response_err = esp_ble_gatts_send_response(gatts_if, param->write.conn_id, param->write.trans_id, status, gatt_rsp);
                if (response_err != ESP_OK){
                    ESP_LOGE(GATTS_TAG, "Send response error\n");
                }
                free(gatt_rsp);
            } else {
                ESP_LOGE(GATTS_TAG, "malloc failed, no resource to send response error\n");
                status = ESP_GATT_NO_RESOURCES;
            }
            if (status != ESP_GATT_OK){
                return;
            }
            memcpy(prepare_write_env->prepare_buf + param->write.offset,
                   param->write.value,
                   param->write.len);
            prepare_write_env->prepare_len += param->write.len;

        }else{
            esp_ble_gatts_send_response(gatts_if, param->write.conn_id, param->write.trans_id, status, NULL);
        }
    }
}

static void example_exec_write_event_env(prepare_type_env_t *prepare_write_env, esp_ble_gatts_cb_param_t *param){
    if (param->exec_write.exec_write_flag == ESP_GATT_PREP_WRITE_EXEC){
        ESP_LOG_BUFFER_HEX(GATTS_TAG, prepare_write_env->prepare_buf, prepare_write_env->prepare_len);
    }else{
        ESP_LOGI(GATTS_TAG,"Prepare write cancel");
    }
    if (prepare_write_env->prepare_buf) {
        free(prepare_write_env->prepare_buf);
        prepare_write_env->prepare_buf = NULL;
    }
    prepare_write_env->prepare_len = 0;
}