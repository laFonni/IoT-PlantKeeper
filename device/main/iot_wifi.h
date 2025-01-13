#pragma once

#include <esp_event.h>
#include <esp_wifi.h>

#define WIFI_SUCCESS 1

extern bool wifi_connected;

void iot_wifi_init(void);
void iot_wifi_start(const char* ssid, const char* pass);
void iot_wifi_stop(void);