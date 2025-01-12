#pragma once

#include <esp_event.h>
#include <esp_wifi.h>

#define WIFI_SUCCESS 1

extern bool wifi_connected;
extern EventGroupHandle_t  wifi_event_group;
extern esp_event_handler_instance_t wifi_handler_event_instance;
extern esp_event_handler_instance_t got_ip_event_instance;

void iot_wifi_init(void);
void iot_wifi_start(const char* ssid, const char* pass);
void iot_wifi_stop(void);