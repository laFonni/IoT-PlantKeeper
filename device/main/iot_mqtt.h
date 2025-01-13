#pragma once

extern bool mqtt_connected;

void iot_mqtt_init(const char* mqtt_url);
void iot_mqtt_deinit(void);