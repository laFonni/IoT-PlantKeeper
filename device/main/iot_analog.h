#pragma once

#include "esp_adc/adc_oneshot.h"

void iot_photoresistor_init(adc_unit_t unit, adc_channel_t channel, adc_atten_t atten);
int32_t iot_photoresistor_get(void);
void iot_photoresistor_deinit(void);