#include "esp_adc/adc_oneshot.h"

#include "iot_analog.h"

static bool calibration_enabled = false;
static adc_oneshot_unit_handle_t adc1_handle;
static adc_cali_handle_t cali_handle = NULL;

void iot_photoresistor_init(adc_unit_t unit, adc_channel_t channel, adc_atten_t atten) {
    adc_oneshot_unit_init_cfg_t unit_cfg = {
        .unit_id = unit,
        .clk_src = ADC_RTC_CLK_SRC_DEFAULT,
    };
    ESP_ERROR_CHECK(adc_oneshot_new_unit(&unit_cfg, &adc1_handle));

    adc_oneshot_chan_cfg_t channel_cfg = {
        .atten = atten,
        .bitwidth = ADC_BITWIDTH_DEFAULT,
    };
    ESP_ERROR_CHECK(adc_oneshot_config_channel(adc1_handle, channel, &channel_cfg));

    adc_cali_line_fitting_config_t cali_config = {
        .unit_id = unit,
        .atten = atten,
        .bitwidth = ADC_BITWIDTH_DEFAULT,
    };

    if (adc_cali_create_scheme_line_fitting(&cali_config, &cali_handle) == ESP_OK) {
        printf("ADC calibration initialized.\n");
        calibration_enabled = true;
    } else {
        printf("ADC calibration could not be initialized. Raw values will be used.\n");
    }
}

int32_t iot_photoresistor_get(adc_channel_t channel) {
    int raw_value;
    ESP_ERROR_CHECK(adc_oneshot_read(adc1_handle, channel, &raw_value));

    if (calibration_enabled) {
        int voltage;
        ESP_ERROR_CHECK(adc_cali_raw_to_voltage(cali_handle, raw_value, &voltage));
        return voltage;
    }
    return raw_value;
}

void iot_photoresistor_deinit() {
    if (calibration_enabled) {
        adc_cali_delete_scheme_line_fitting(cali_handle);
    }
    ESP_ERROR_CHECK(adc_oneshot_del_unit(adc1_handle));
}
