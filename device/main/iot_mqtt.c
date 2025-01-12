#include <inttypes.h>
#include <esp_event_base.h>
#include <mqtt_client.h>

#include "iot_mqtt.h"

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

// void iot_mqtt_init(const char* mqtt_url) {
//     char mqtt_full_url[32] = "mqtt://";
//     strncat(mqtt_full_url, mqtt_url, strlen(mqtt_url) + 1);
//     printf("Full MQTT URL: %s\n", mqtt_full_url);
//     esp_mqtt_client_config_t mqtt_cfg = {
//         .broker.address.uri = mqtt_full_url,
//     };

//     esp_mqtt_client_handle_t client = esp_mqtt_client_init(&mqtt_cfg);
//     esp_mqtt_client_register_event(client, ESP_EVENT_ANY_ID, mqtt_event_handler, NULL);
//     // esp_mqtt_client_start(client);
// }

// void iot_mqtt_start(const char* mqtt_url) {
//     char mqtt_full_url[32] = "mqtt://";
//     strncat(mqtt_full_url, mqtt_url, strlen(mqtt_url) + 1);
//     printf("Full MQTT URL: %s\n", mqtt_full_url);
//     ESP_ERROR_CHECK(esp_mqtt_client_set_uri(mqtt_client, mqtt_full_url));
//     esp_mqtt_client_start(mqtt_client);
// }

// void iot_mqtt_stop(void) {
//         // esp_mqtt_client_disconnect(mqtt_client);
//     // if (mqtt_publish_task_handle != NULL) {
//     //     vTaskDelete(mqtt_publish_task_handle);
//     //     mqtt_publish_task_handle = NULL;
//     // }
//     esp_mqtt_client_stop(mqtt_client);
// }

void iot_mqtt_deinit() {
    if (mqtt_publish_task_handle != NULL) {
        vTaskDelete(mqtt_publish_task_handle);
        mqtt_publish_task_handle = NULL;
    }
    if (mqtt_client != NULL) {
        // esp_mqtt_client_disconnect(mqtt_client);
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

        if (mqtt_publish_task_handle == NULL) {
            xTaskCreate(mqtt_publish_task, "publish", 2048, NULL, 1, &mqtt_publish_task_handle);
        }
    } else if (event_id == MQTT_EVENT_PUBLISHED) {
        printf("Opublikowano wiadomość, id: %d\n", event->msg_id);
    } else if (event_id == MQTT_EVENT_DISCONNECTED) {
        mqtt_connected = false;
        printf("Połączenie z brokerem przerwane.\n");

        if (mqtt_publish_task_handle != NULL) {
            vTaskDelete(mqtt_publish_task_handle);
            mqtt_publish_task_handle = NULL;
        }
    }
}

static void mqtt_publish_task(void* pvParameters) {
    int i = 0;
    while (true) {
        if (i % 5 == 0) {
            esp_mqtt_client_publish(mqtt_client, SOIL_MOISTURE_TOPIC_SUFFIX, "data", 0, 1, 0);
        } else if (i % 5 == 1) {
            esp_mqtt_client_publish(mqtt_client, TEMPERATURE_TOPIC_SUFFIX, "data", 0, 1, 0);
        } else if (i % 5 == 2) {
            esp_mqtt_client_publish(mqtt_client, WATER_PUMP_TOPIC_SUFFIX, "data", 0, 1, 0);
        } else if (i % 5 == 3) {
            esp_mqtt_client_publish(mqtt_client, LIGHT_SENSOR_TOPIC_SUFFIX, "data", 0, 1, 0);
        } else if (i % 5 == 4) {
            esp_mqtt_client_publish(mqtt_client, LIGHT_EMITTER_TOPIC_SUFFIX, "data", 0, 1, 0);
        }
        vTaskDelay(1000 / portTICK_PERIOD_MS);
    }
}