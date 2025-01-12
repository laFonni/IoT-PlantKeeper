#pragma once

extern bool mqtt_connected;
extern char mqtt_url[64];

void iot_mqtt_init(const char* mqtt_url);
void iot_mqtt_deinit(void);