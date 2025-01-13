#include <string.h>
#include <nvs.h>
#include <nvs_flash.h>

#include "iot_wifi.h"
#include "iot_nvs.h"
#include "iot_mqtt.h"
#include "iot_time.h"

bool wifi_connected = false;
static esp_event_handler_instance_t wifi_handler_event_instance;
static esp_event_handler_instance_t got_ip_event_instance;

static void wifi_event_handler(void* arg, esp_event_base_t event_base, int32_t event_id, void* event_data) {
    if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_START) {
        printf("Łączenie z Access Point...\n");
        esp_wifi_connect();
    } else if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_CONNECTED) {
        wifi_connected = true;

        printf("Połączono z Access Point.\n");
    } else if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_DISCONNECTED) {
        wifi_connected = false;

        iot_mqtt_deinit();

		printf("Łączenie z Access Point...\n");
        esp_wifi_connect();
    }
}

static void ip_event_handler(void* arg, esp_event_base_t event_base, int32_t event_id, void* event_data) {
    if (event_base == IP_EVENT && event_id == IP_EVENT_STA_GOT_IP) {
        ip_event_got_ip_t* event = (ip_event_got_ip_t*) event_data;
        printf("Uzyskany adres IP: " IPSTR "\n", IP2STR(&event->ip_info.ip));

        wait_for_time_sync();

        char mqtt_uri[64];
        get_mqtt_uri(mqtt_uri);
        iot_mqtt_init(mqtt_uri);
    }
}

void iot_wifi_init(void) {
    ESP_ERROR_CHECK(esp_netif_init());
    ESP_ERROR_CHECK(esp_event_loop_create_default());
    esp_netif_create_default_wifi_sta();
    wifi_init_config_t initial_wifi_config = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_wifi_init(&initial_wifi_config));

    ESP_ERROR_CHECK(esp_event_handler_instance_register(WIFI_EVENT, ESP_EVENT_ANY_ID, &wifi_event_handler, NULL, &wifi_handler_event_instance));
    ESP_ERROR_CHECK(esp_event_handler_instance_register(IP_EVENT, IP_EVENT_STA_GOT_IP, &ip_event_handler, NULL, &got_ip_event_instance));
    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA));
}

void iot_wifi_start(const char* ssid, const char* pass) {
    wifi_config_t wifi_config = {
        .sta = {
            .ssid = "",
            .password = "",
	        .threshold.authmode = WIFI_AUTH_WPA2_PSK,
            .pmf_cfg = {
                .capable = true,
                .required = false
            },
        },
    };
    strncpy((char *) wifi_config.sta.ssid, ssid, sizeof (wifi_config.sta.ssid));
    strncpy((char *) wifi_config.sta.password, pass, sizeof (wifi_config.sta.password));

    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA));
    ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_STA, &wifi_config));
    ESP_ERROR_CHECK(esp_wifi_start());
}

void iot_wifi_stop(void) {
    esp_wifi_stop();
}