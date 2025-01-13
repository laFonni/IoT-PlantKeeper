#include <time.h>
#include <sys/time.h>
#include <esp_sntp.h>

#include "iot_time.h"
#include "iot_reconfigure.h"

bool time_synchronized = false;

void initialize_sntp(void) {
    printf("Inicjalizowanie SNTP...\n");
    esp_sntp_setoperatingmode(SNTP_OPMODE_POLL);
    esp_sntp_setservername(0, "pool.ntp.org");
    esp_sntp_init();
}

void wait_for_time_sync(void) {
    time_synchronized = false;
    esp_sntp_restart();
    time_t now = 0;
    struct tm timeinfo = { 0 };

    while (timeinfo.tm_year < (2025 - 1900)) {
        if (setup_mode) {
            return;
        }
        printf("Synchronizowanie czasu...\n");
        time(&now);
        localtime_r(&now, &timeinfo);
        vTaskDelay(2000 / portTICK_PERIOD_MS);
    }
    esp_sntp_stop();
    time_synchronized = true;
    printf("Czas zsynchronizowany: %s", asctime(&timeinfo));
}

void get_timestamp(char* timestamp) {
    time_t now;
    time(&now);

    struct tm timeinfo;
    localtime_r(&now, &timeinfo);
    strftime(timestamp, 64, "%Y-%m-%d %H:%M:%S", &timeinfo);
}