#ifndef NVS_MANAGER_H
#define NVS_MANAGER_H

#include <esp_err.h>
#include <stdint.h>
#include <stdbool.h>
#include <time.h>

// Inizializza la NVS
esp_err_t nvs_manager_init(void);

// Legge un valore intero dalla NVS
esp_err_t nvs_manager_get_int(uint64_t key, int *value);

// Salva un valore intero nella NVS
esp_err_t nvs_manager_set_int(uint64_t key, int value);

// Legge un valore booleano dalla NVS
esp_err_t nvs_manager_get_bool(uint64_t key, bool *value);

// Salva un valore booleano nella NVS
esp_err_t nvs_manager_set_bool(uint64_t key, bool value);

// Legge un valore float dalla NVS
esp_err_t nvs_manager_get_float(uint64_t key, float *value);

// Salva un valore float nella NVS
esp_err_t nvs_manager_set_float(uint64_t key, float value);

// Legge un valore timestamp dalla NVS
esp_err_t nvs_manager_get_timestamp(uint64_t key, time_t *value);

// Salva un valore timestamp nella NVS
esp_err_t nvs_manager_set_timestamp(uint64_t key, time_t value);

// Legge un valore string nella NVS
esp_err_t nvs_manager_get_string(uint64_t key, char *value, size_t max_len);

// Salva un valore string nella NVS
esp_err_t nvs_manager_set_string(uint64_t key, const char *value);

// Cancella tutti i dati nella NVS
esp_err_t nvs_manager_erase_all(void);

#endif // NVS_MANAGER_H