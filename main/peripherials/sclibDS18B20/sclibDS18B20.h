#ifndef sclibDS18B20_H
#define sclibDS18B20_H

#include <stdbool.h>
#include <stdint.h>
#include <esp_log.h>
#include <driver/gpio.h>
#include <onewire_bus.h>
#include <ds18b20.h>

#define ONEWIRE_BUS_GPIO    33
#define ONEWIRE_MAX_DS18B20 1

// Inizializza il bus e trova il primo sensore DS18B20
bool ds18b20_init(void);

// Legge la temperatura dal primo sensore trovato
bool ds18b20_read_temperature(float *temp);

#endif // sclibDS18B20_H