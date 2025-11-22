#include "sclibDS18B20.h"
#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

static onewire_bus_handle_t ds18b20_bus = NULL;
static ds18b20_device_handle_t ds18b20_handle = NULL;
static onewire_device_address_t ds18b20_address = 0;

// Inizializzazione bus e ricerca primo sensore DS18B20
bool ds18b20_init(void) {
    onewire_bus_config_t bus_config = {
        .bus_gpio_num = ONEWIRE_BUS_GPIO,
        .flags = {.en_pull_up = true}
    };
    onewire_bus_rmt_config_t rmt_config = {
        .max_rx_bytes = 10
    };
    if (ESP_OK != onewire_new_bus_rmt(&bus_config, &rmt_config, &ds18b20_bus)) return false;

    onewire_device_iter_handle_t iter = NULL;
    if (ESP_OK != onewire_new_device_iter(ds18b20_bus, &iter)) return false;
    onewire_device_t found;
    bool ok = false;
    while (onewire_device_iter_get_next(iter, &found) == ESP_OK) {
        ds18b20_config_t cfg = {};
        ds18b20_device_handle_t handle = NULL;
        if (ds18b20_new_device_from_enumeration(&found, &cfg, &handle) == ESP_OK) {
            ds18b20_handle = handle;
            ds18b20_address = found.address;
            ok = true;
            break;
        }
    }
    onewire_del_device_iter(iter);
    return ok;
}

// Leggi temperatura dal primo DS18B20 trovato
bool ds18b20_read_temperature(float *temp) {
    if (!ds18b20_handle) return false;
    if (ds18b20_trigger_temperature_conversion(ds18b20_handle) != ESP_OK) return false;
    vTaskDelay(pdMS_TO_TICKS(750)); // attesa conversione (max 750ms per 12bit)
    float t = 0;
    if (ds18b20_get_temperature(ds18b20_handle, &t) != ESP_OK) return false;
    if (temp) *temp = t;
    return true;
}
#include "DS18B20.h"
#include <stdbool.h>
#include <stdint.h>
#include <stddef.h>
#include <string.h>

#ifndef TAG
#define TAG "DS18B20"
#endif


