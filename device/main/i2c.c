#include <esp_rom_sys.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "i2c.h"

int32_t t_fine;
int32_t bmp280_compensate_T_int32(int32_t adc_T) {
    int32_t var1, var2, T;
    var1 = ((((adc_T>>3) - ((int32_t) calib.dig_T1<<1))) * ((int32_t) calib.dig_T2)) >> 11;
    var2 = (((((adc_T>>4) - ((int32_t) calib.dig_T1)) * ((adc_T>>4) - ((int32_t) calib.dig_T1))) >> 12) * ((int32_t) calib.dig_T3)) >> 14;
    t_fine = var1 + var2;
    T = (t_fine * 5 + 128) >> 8;
    return T;
}

calib_t calib;

i2c_device_t device;

int get_calibration_params(i2c_device_t i2c) {
    printf("Odczyt kalibracyjny: wysłanie adresu\n");
    i2c_start(&i2c);
    if (!i2c_write_byte(&i2c, (0x76 << 1) | 0x0)) {
        printf("Odczyt kalibracyjny: Brak ACK\n");
        i2c_stop(&i2c);
        return 1;
    }
    printf("Odczyt kalibracyjny: otrzymano ACK\n");

    printf("Odczyt temperatury: wysłanie adresu rejestru 88\n");
    if (!i2c_write_byte(&i2c, 0x88)) {
        printf("Odczyt kalibracyjny: Brak ACK\n");
        i2c_stop(&i2c);
        return 1;
    }
    printf("Odczyt kalibracyjny: otrzymano ACK\n");

    // Właściwy odczyt
    printf("Odczyt kalibracyjny: ponowne wysłanie adresu\n");
    i2c_start(&i2c);
    if (!i2c_write_byte(&i2c, (0x76 << 1) | 0x1)) {
        printf("Odczyt kalibracyjny: Brak ACK\n");
        i2c_stop(&i2c);
        return 1;
    }
    printf("Odczyt kalibracyjny: otrzymano ACK\n");

    printf("Odczyt kalibracyjny: czytanie wartości\n");
    calib.dig_T1 = i2c_read_byte(&i2c, true);
    calib.dig_T1 |= i2c_read_byte(&i2c, true) << 8;

    calib.dig_T2 = i2c_read_byte(&i2c, true);
    calib.dig_T2 |= i2c_read_byte(&i2c, true) << 8;

    calib.dig_T3 = i2c_read_byte(&i2c, true);
    calib.dig_T3 |= i2c_read_byte(&i2c, true) << 8;

    calib.dig_P1 = i2c_read_byte(&i2c, true);
    calib.dig_P1 |= i2c_read_byte(&i2c, true) << 8;

    calib.dig_P2 = i2c_read_byte(&i2c, true);
    calib.dig_P2 |= i2c_read_byte(&i2c, true) << 8;

    calib.dig_P3 = i2c_read_byte(&i2c, true);
    calib.dig_P3 |= i2c_read_byte(&i2c, true) << 8;

    calib.dig_P4 = i2c_read_byte(&i2c, true);
    calib.dig_P4 |= i2c_read_byte(&i2c, true) << 8;

    calib.dig_P5 = i2c_read_byte(&i2c, true);
    calib.dig_P5 |= i2c_read_byte(&i2c, true) << 8;

    calib.dig_P6 = i2c_read_byte(&i2c, true);
    calib.dig_P6 |= i2c_read_byte(&i2c, true) << 8;

    calib.dig_P7 = i2c_read_byte(&i2c, true);
    calib.dig_P7 |= i2c_read_byte(&i2c, true) << 8;

    calib.dig_P8 = i2c_read_byte(&i2c, true);
    calib.dig_P8 |= i2c_read_byte(&i2c, true) << 8;

    calib.dig_P9 = i2c_read_byte(&i2c, true);
    calib.dig_P9 |= i2c_read_byte(&i2c, false) << 8;
    
    i2c_stop(&i2c);
    printf("T: %d %d %d\n", calib.dig_T1, calib.dig_T2, calib.dig_T3);
    printf("P: %d %d %d %d %d %d %d %d %d\n",
            calib.dig_P1, calib.dig_P2, calib.dig_P3,
            calib.dig_P4, calib.dig_P5, calib.dig_P6,
            calib.dig_P7, calib.dig_P8, calib.dig_P9);
    return 0;
}

int trigger_measurment(i2c_device_t i2c) {
    printf("Wywoływanie pomiaru\n");
    i2c_start(&i2c);
    if (!i2c_write_byte(&i2c, (0x76 << 1) | 0x0)) {
        printf("Wywoływanie pomiaru: Brak ACK\n");
        i2c_stop(&i2c);
        return 1;
    }
    printf("Wysyłanie adresu rejestru\n");
    if (!i2c_write_byte(&i2c, 0xF4)) {
        printf("Wywoływanie pomiaru: Brak ACK\n");
        i2c_stop(&i2c);
        return 1;
    }
    printf("Wysyłanie wartości 0x25\n");
    if (!i2c_write_byte(&i2c, 0x25)) {
        printf("Wywoływanie pomiaru: Brak ACK\n");
        i2c_stop(&i2c);
        return 1;
    }
    i2c_stop(&i2c);

    bool measurement_in_progress = true;
    while (measurement_in_progress) {
        i2c_start(&i2c);
        if (!i2c_write_byte(&i2c, (0x76 << 1) | 0x0)) {
            printf("Odczyt statusu: Brak ACK\n");
            i2c_stop(&i2c);
            return 1;
        }
        if (!i2c_write_byte(&i2c, 0xF3)) {
            printf("Odczyt statusu: Brak ACK\n");
            i2c_stop(&i2c);
            return 1;
        }

        i2c_start(&i2c);
        if (!i2c_write_byte(&i2c, (0x76 << 1) | 0x1)) {
            printf("Odczyt statusu: Brak ACK\n");
            i2c_stop(&i2c);
            return 1;
        }
        uint8_t data_F3 = i2c_read_byte(&i2c, false);
        i2c_stop(&i2c);
        printf("Status %x\n", data_F3);
        if ((data_F3 & 0b00001000) == 0 && (data_F3 & 0b00000001) == 0) {
            measurement_in_progress = false;
        }

        vTaskDelay(100 / portTICK_PERIOD_MS);
    }
    return 0;
}

int read_temp(i2c_device_t i2c, int32_t *result) {
    printf("Odczyt temperatury: wysłanie adresu\n");
    i2c_start(&i2c);
    if (!i2c_write_byte(&i2c, (0x76 << 1) | 0x0)) {
        printf("Odczyt temperatury: Brak ACK\n");
        i2c_stop(&i2c);
        return 1;
    }
    printf("Odczyt temperatury: otrzymano ACK\n");

    printf("Odczyt temperatury: wysłanie adresu rejestru F6\n");
    if (!i2c_write_byte(&i2c, 0xF6)) {
        printf("Odczyt temperatury: Brak ACK\n");
        i2c_stop(&i2c);
        return 1;
    }
    printf("Odczyt temperatury: otrzymano ACK\n");

    // Właściwy odczyt
    printf("Odczyt temperatury: ponowne wysłanie adresu\n");
    i2c_start(&i2c);
    if (!i2c_write_byte(&i2c, (0x76 << 1) | 0x1)) {
        printf("Odczyt temperatury: Brak ACK\n");
        i2c_stop(&i2c);
        return 1;
    }
    printf("Odczyt temperatury: otrzymano ACK\n");

    printf("Odczyt temperatury: czytanie wartości\n");
    uint8_t data_F6 = i2c_read_byte(&i2c, true);
    uint8_t data_F7 = i2c_read_byte(&i2c, true);
    uint8_t data_F8 = i2c_read_byte(&i2c, true);
    uint8_t data_F9 = i2c_read_byte(&i2c, true);
    uint8_t data_FA = i2c_read_byte(&i2c, true);
    uint8_t data_FB = i2c_read_byte(&i2c, true);
    uint8_t data_FC = i2c_read_byte(&i2c, false);
    i2c_stop(&i2c);
    printf("%x %x %x %x %x %x %x\n", data_F6, data_F7, data_F8, data_F9, data_FA, data_FB, data_FC);
    *result = (data_FA << 12) | (data_FB << 4) | (data_FC >> 4);
    return 0;
}

double get_temp_deg(i2c_device_t i2c) {
    trigger_measurment(i2c);
    int32_t result;
    read_temp(i2c, &result);
    int32_t temperature = bmp280_compensate_T_int32(result);
    return temperature / 100.0;
}

static void i2c_delay() {
    esp_rom_delay_us(I2C_DELAY_US);
}

static void i2c_set_sda(i2c_device_t *i2c, bool level) {
    gpio_set_level(i2c->sda_pin, level);
}

static bool i2c_get_sda(i2c_device_t *i2c) {
    return gpio_get_level(i2c->sda_pin);
}

static void i2c_set_scl(i2c_device_t *i2c, bool level) {
    gpio_set_level(i2c->scl_pin, level);
}

void i2c_init(i2c_device_t *i2c, gpio_num_t sda, gpio_num_t scl, const char *name) {
    i2c->sda_pin = sda;
    i2c->scl_pin = scl;
    i2c->name = name;

    gpio_set_direction(sda, GPIO_MODE_INPUT_OUTPUT_OD);
    gpio_set_pull_mode(sda, GPIO_PULLUP_ONLY);

    gpio_set_direction(scl, GPIO_MODE_INPUT_OUTPUT_OD);
    gpio_set_pull_mode(scl, GPIO_PULLUP_ONLY);

    printf("Urządzenie %s zainicjalizowane; SDA: %d, SCL: %d\n", name, sda, scl);
}

void i2c_start(i2c_device_t *i2c) {
    i2c_set_sda(i2c, 1);
    i2c_set_scl(i2c, 1);
    i2c_delay();
    i2c_set_sda(i2c, 0);
    i2c_delay();
    i2c_set_scl(i2c, 0);
}

void i2c_stop(i2c_device_t *i2c) {
    i2c_set_sda(i2c, 0);
    i2c_set_scl(i2c, 1);
    i2c_delay();
    i2c_set_sda(i2c, 1);
    i2c_delay();
}

bool i2c_write_byte(i2c_device_t *i2c, uint8_t byte) {
    for (int i = 0; i < 8; i++) {
        i2c_set_sda(i2c, (byte & 0x80) != 0);
        byte <<= 1;
        i2c_delay();

        i2c_set_scl(i2c, 1);
        i2c_delay();
        i2c_set_scl(i2c, 0);
        i2c_delay();
    }

    gpio_set_direction(i2c->sda_pin, GPIO_MODE_INPUT);
    i2c_delay();

    i2c_set_scl(i2c, 1);
    i2c_delay();
    bool ack = !i2c_get_sda(i2c);
    i2c_set_scl(i2c, 0);
    i2c_delay();

    gpio_set_direction(i2c->sda_pin, GPIO_MODE_INPUT_OUTPUT_OD);

    return ack;
}

uint8_t i2c_read_byte(i2c_device_t *i2c, bool ack) {
    uint8_t byte = 0;

    gpio_set_direction(i2c->sda_pin, GPIO_MODE_INPUT);

    for (int i = 0; i < 8; i++) {
        byte <<= 1;

        i2c_set_scl(i2c, 1);
        i2c_delay();

        if (i2c_get_sda(i2c)) {
            byte |= 0x01;
        }

        i2c_set_scl(i2c, 0);
        i2c_delay();
    }

    gpio_set_direction(i2c->sda_pin, GPIO_MODE_INPUT_OUTPUT_OD);

    i2c_set_sda(i2c, !ack);
    i2c_delay();

    i2c_set_scl(i2c, 1);
    i2c_delay();
    i2c_set_scl(i2c, 0);
    i2c_delay();

    return byte;
}
