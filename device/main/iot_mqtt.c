#include <inttypes.h>
#include <esp_event_base.h>
#include <mqtt_client.h>

#include "iot_mqtt.h"
#include "i2c.h"
#include "iot_analog.h"
#include "iot_moisture.h"
#include "iot_led.h"
#include "iot_time.h"
#include "iot_mac.h"
#include "topics.h"

bool mqtt_connected = false;
static esp_mqtt_client_handle_t mqtt_client = NULL;
static TaskHandle_t mqtt_publish_task_handle = NULL;

static void mqtt_event_handler(void *handler_args, esp_event_base_t event_base, int32_t event_id, void *event_data);
static void mqtt_publish_task(void* pvParameters);

void iot_mqtt_init(const char* mqtt_uri) {
    esp_mqtt_client_config_t mqtt_config = {
        .broker.address.uri = mqtt_uri
    };

    mqtt_client = esp_mqtt_client_init(&mqtt_config);
    esp_mqtt_client_register_event(mqtt_client, ESP_EVENT_ANY_ID, mqtt_event_handler, NULL);
    esp_mqtt_client_start(mqtt_client);
}

void iot_mqtt_deinit() {
    if (mqtt_publish_task_handle != NULL) {
        vTaskDelete(mqtt_publish_task_handle);
        mqtt_publish_task_handle = NULL;
    }
    if (mqtt_client != NULL) {
        esp_mqtt_client_stop(mqtt_client);
        esp_mqtt_client_destroy(mqtt_client);
        mqtt_client = NULL;
    }
}

static void mqtt_event_handler(void *handler_args, esp_event_base_t event_base, int32_t event_id, void *event_data) {
    esp_mqtt_event_handle_t event = event_data;
    mqtt_client = event->client;
    if (event_id == MQTT_EVENT_CONNECTED) {
        mqtt_connected = true;
        printf("Połączono z brokerem MQTT.\n");

        char topic_lamp[64], topic_pump[64];
        sprintf(topic_lamp, "%s/lamp", mac_address);
        sprintf(topic_pump, "%s/pump", mac_address);
        esp_mqtt_client_subscribe(mqtt_client, topic_lamp, 1);
        esp_mqtt_client_subscribe(mqtt_client, topic_pump, 1);

        if (mqtt_publish_task_handle == NULL) {
            xTaskCreate(mqtt_publish_task, "publish", 4096, NULL, 1, &mqtt_publish_task_handle);
        }
    } else if (event_id == MQTT_EVENT_DISCONNECTED) {
        if (mqtt_connected) {
            mqtt_connected = false;
            printf("Połączenie z brokerem MQTT przerwane.\n");
        }

        if (mqtt_publish_task_handle != NULL) {
            vTaskDelete(mqtt_publish_task_handle);
            mqtt_publish_task_handle = NULL;
        }
    } else if (event_id == MQTT_EVENT_PUBLISHED) {
        printf("Opublikowano wiadomość\n");
    } else if (event_id == MQTT_EVENT_DATA) {
        printf("Otrzymano dane:\n");
        printf("Topic: %.*s\n", event->topic_len, event->topic);
        printf("Dane: %.*s\n", event->data_len, event->data);

        char topic_lamp[64], topic_pump[64];
        sprintf(topic_lamp, "%s/lamp", mac_address);
        sprintf(topic_pump, "%s/pump", mac_address);

        if (strncmp(topic_lamp, event->topic, event->topic_len) == 0) {
            if (strncmp("1", event->data, event->data_len) == 0) {
                iot_light_on();
            } else {
                iot_light_off();
            }
        } else if (strncmp(topic_pump, event->topic, event->topic_len) == 0) {
            if (strncmp("1", event->data, event->data_len) == 0) {
                printf("Pump turned on\n");
            } else {
                printf("Pump turned off\n");
            }
        }
    }
}

static void mqtt_publish_task(void* pvParameters) {
    while (true) {
        char timestamp[64];
        get_timestamp(timestamp);

        int32_t light_level = iot_photoresistor_get();
        bool is_wet = is_soil_wet();
        double temperature = get_temp_deg(device);

        char payload[256], topic[64];;

        sprintf(payload, "{\"value\": %ld, \"timestamp\": \"%s\"}", light_level, timestamp);
        sprintf(topic, "%s/light_sensor", mac_address);
        esp_mqtt_client_publish(mqtt_client, topic, payload, 0, 1, 0);

        sprintf(payload, "{\"value\": %d, \"timestamp\": \"%s\"}", is_wet, timestamp);
        sprintf(topic, "%s/soil_moisture", mac_address);
        esp_mqtt_client_publish(mqtt_client, topic, payload, 0, 1, 0);

        sprintf(payload, "{\"value\": %f, \"timestamp\": \"%s\"}", temperature, timestamp);
        sprintf(topic, "%s/temperature", mac_address);
        esp_mqtt_client_publish(mqtt_client, topic, payload, 0, 1, 0);

        vTaskDelay(5000 / portTICK_PERIOD_MS);
    }
}