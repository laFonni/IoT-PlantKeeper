#include <string.h>
#include <esp_mac.h>

#include "iot_mac.h"

char mac_address[18];

void get_mac_address(void) {
    uint8_t mac[6];
    esp_err_t ret = esp_efuse_mac_get_default(mac);
    if (ret != ESP_OK) {
        printf("Nie udało się uzyskać adresu MAC\n");
    }

    sprintf(mac_address, "%02X:%02X:%02X:%02X:%02X:%02X", 
            mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
    
    printf("MAC: %s\n", mac_address);
}