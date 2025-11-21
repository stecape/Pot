#include "nvs_manager.h"
#include <nvs_flash.h>
#include <nvs.h>
#include <esp_log.h>
#include <time.h>    // Per il tipo time_t
#include <string.h>  // Per la funzione snprintf

#define NVS_PARTITION_NAME "hmi" // Nome della partizione NVS
static const char *TAG = "NVS_MANAGER";
static const char *NVS_NAMESPACE = "storage"; // Nome dello spazio dei nomi NVS
// Inizializza la NVS
// Questa funzione deve essere chiamata all'inizio del programma
// per inizializzare la memoria non volatile
// Se la partizione NVS non è disponibile o è di una versione più recente,
// viene eseguita l'operazione di cancellazione della partizione
// e viene inizializzata nuovamente
esp_err_t nvs_manager_init() {
    esp_err_t err = nvs_flash_init_partition(NVS_PARTITION_NAME);
    if (err == ESP_ERR_NVS_NO_FREE_PAGES || err == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        ESP_LOGW(TAG, "NVS partition needs to be erased");
        ESP_ERROR_CHECK(nvs_flash_erase_partition(NVS_PARTITION_NAME));
        err = nvs_flash_init_partition(NVS_PARTITION_NAME);
    }
    if (err == ESP_OK) {
        ESP_LOGI(TAG, "NVS initialized successfully");
    } else {
        ESP_LOGE(TAG, "Failed to initialize NVS: %s", esp_err_to_name(err));
    }
    return err;
}

// Funzione per leggere un valore int dalla NVS
esp_err_t nvs_manager_get_int(uint64_t key, int *value) {
    char key_str[16];
    snprintf(key_str, sizeof(key_str), "%llx", key); // Convert key to hex string

    nvs_handle_t handle;
    esp_err_t err = nvs_open_from_partition(NVS_PARTITION_NAME, NVS_NAMESPACE, NVS_READONLY, &handle);
    if (err != ESP_OK) {
        //ESP_LOGE(TAG, "Failed to open NVS: %s", esp_err_to_name(err));
        return err;
    }

    int32_t temp_value;
    err = nvs_get_i32(handle, key_str, &temp_value); // Use key_str
    if (err == ESP_OK) {
        *value = (int)temp_value;
        //ESP_LOGI(TAG, "Read key '%s': %d", key_str, *value);
    } else if (err == ESP_ERR_NVS_NOT_FOUND) {
        ESP_LOGW(TAG, "Key '%s' not found", key_str);
        //*value = 0; // Default value if key not found
    } else {
        ESP_LOGE(TAG, "Failed to read key '%s': %s", key_str, esp_err_to_name(err));
    }

    nvs_close(handle);
    return err;
}

// Funzione per scrivere un valore int nella NVS
esp_err_t nvs_manager_set_int(uint64_t key, int value) {
    char key_str[16];
    snprintf(key_str, sizeof(key_str), "%llx", key); // Convert key to hex string

    int current_value;
    esp_err_t err = nvs_manager_get_int(key, &current_value);
    if (err == ESP_OK && current_value == value) {
        ESP_LOGI(TAG, "Key '%s' already has the value %d, skipping write", key_str, value);
        return ESP_OK; // Evita la scrittura se il valore è già uguale
    }

    nvs_handle_t handle;
    err = nvs_open_from_partition(NVS_PARTITION_NAME, NVS_NAMESPACE, NVS_READWRITE, &handle);
    if (err != ESP_OK) {
        //ESP_LOGE(TAG, "Failed to open NVS: %s", esp_err_to_name(err));
        return err;
    }

    int32_t temp_value = (int32_t)value;
    err = nvs_set_i32(handle, key_str, temp_value); // Use key_str
    if (err == ESP_OK) {
        ESP_LOGI(TAG, "Saved key '%s': %d", key_str, value);
        nvs_commit(handle);
    } else {
        ESP_LOGE(TAG, "Failed to save key '%s': %s", key_str, esp_err_to_name(err));
    }

    nvs_close(handle);
    return err;
}

// Funzione per leggere un valore bool dalla NVS
esp_err_t nvs_manager_get_bool(uint64_t key, bool *value) {
    char key_str[16];
    snprintf(key_str, sizeof(key_str), "%llx", key); // Convert key to hex string

    int temp;
    esp_err_t err = nvs_manager_get_int(key, &temp);
    if (err == ESP_OK) {
        *value = (temp != 0);
    } else {
        ESP_LOGW(TAG, "Failed to read bool key '%s', defaulting to false", key_str);
        //*value = false; // Default value if key not found or error
    }
    return err;
}

// Funzione per scrivere un valore bool nella NVS
esp_err_t nvs_manager_set_bool(uint64_t key, bool value) {
    char key_str[16];
    snprintf(key_str, sizeof(key_str), "%llx", key); // Convert key to hex string

    bool current_value;
    esp_err_t err = nvs_manager_get_bool(key, &current_value);
    if (err == ESP_OK && current_value == value) {
        ESP_LOGI(TAG, "Key '%s' already has the value %s, skipping write", key_str, value ? "true" : "false");
        return ESP_OK; // Evita la scrittura se il valore è già uguale
    }

    return nvs_manager_set_int(key, value ? 1 : 0);
}

// Funzione per scrivere un valore float nella NVS
esp_err_t nvs_manager_set_float(uint64_t key, float value) {
    char key_str[16];
    snprintf(key_str, sizeof(key_str), "%llx", key); // Convert key to hex string

    float current_value;
    esp_err_t err = nvs_manager_get_float(key, &current_value);
    ESP_LOGI(TAG, "Writing float on NVS result: %s", esp_err_to_name(err));

    if (err == ESP_OK && current_value == value) {
        ESP_LOGI(TAG, "Key '%s' already has the value %f, skipping write", key_str, value);
        return ESP_OK; // Evita la scrittura se il valore è già uguale
    }

    nvs_handle_t handle;
    err = nvs_open_from_partition(NVS_PARTITION_NAME, NVS_NAMESPACE, NVS_READWRITE, &handle);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "Failed to open NVS: %s", esp_err_to_name(err));
        return err;
    }

    err = nvs_set_blob(handle, key_str, &value, sizeof(value)); // Use key_str
    if (err == ESP_OK) {
        ESP_LOGI(TAG, "Saved float key '%s': %f", key_str, value);
        nvs_commit(handle);
    } else {
        ESP_LOGE(TAG, "Failed to save float key '%s': %s", key_str, esp_err_to_name(err));
    }

    nvs_close(handle);
    return err;
}

// Funzione per leggere un valore float dalla NVS
esp_err_t nvs_manager_get_float(uint64_t key, float *value) {
    char key_str[16];
    snprintf(key_str, sizeof(key_str), "%llx", key); // Convert key to hex string

    nvs_handle_t handle;
    esp_err_t err = nvs_open_from_partition(NVS_PARTITION_NAME, NVS_NAMESPACE, NVS_READONLY, &handle);
    if (err != ESP_OK) {
        //ESP_LOGE(TAG, "Failed to open NVS: %s", esp_err_to_name(err));
        return err;
    }

    size_t required_size = sizeof(*value);
    err = nvs_get_blob(handle, key_str, value, &required_size); // Use key_str
    if (err == ESP_OK) {
        //ESP_LOGI(TAG, "Read float key '%s': %f", key_str, *value);
    } else if (err == ESP_ERR_NVS_NOT_FOUND) {
        ESP_LOGW(TAG, "Float key '%s' not found", key_str);
        //*value = 0.0f; // Default value if key not found
    } else {
        ESP_LOGE(TAG, "Failed to read float key '%s': %s", key_str, esp_err_to_name(err));
    }

    nvs_close(handle);
    return err;
}

// Funzione per scrivere un valore timestamp nella NVS
esp_err_t nvs_manager_set_timestamp(uint64_t key, time_t value) {
    char key_str[16];
    snprintf(key_str, sizeof(key_str), "%llx", key); // Convert key to hex string

    time_t current_value;
    esp_err_t err = nvs_manager_get_timestamp(key, &current_value);
    if (err == ESP_OK && current_value == value) {
        ESP_LOGI(TAG, "Key '%s' already has the timestamp %lld, skipping write", key_str, (long long)value);
        return ESP_OK; // Evita la scrittura se il valore è già uguale
    }

    nvs_handle_t handle;
    err = nvs_open_from_partition(NVS_PARTITION_NAME, NVS_NAMESPACE, NVS_READWRITE, &handle);
    if (err != ESP_OK) {
        //ESP_LOGE(TAG, "Failed to open NVS: %s", esp_err_to_name(err));
        return err;
    }

    err = nvs_set_blob(handle, key_str, &value, sizeof(value)); // Use key_str
    if (err == ESP_OK) {
        ESP_LOGI(TAG, "Saved timestamp key '%s': %lld", key_str, (long long)value);
        nvs_commit(handle);
    } else {
        ESP_LOGE(TAG, "Failed to save timestamp key '%s': %s", key_str, esp_err_to_name(err));
    }

    nvs_close(handle);
    return err;
}

// Funzione per leggere un valore timestamp dalla NVS
esp_err_t nvs_manager_get_timestamp(uint64_t key, time_t *value) {
    char key_str[16];
    snprintf(key_str, sizeof(key_str), "%llx", key); // Convert key to hex string

    nvs_handle_t handle;
    esp_err_t err = nvs_open_from_partition(NVS_PARTITION_NAME, NVS_NAMESPACE, NVS_READONLY, &handle);
    if (err != ESP_OK) {
        //ESP_LOGE(TAG, "Failed to open NVS: %s", esp_err_to_name(err));
        return err;
    }

    size_t required_size = sizeof(*value);
    err = nvs_get_blob(handle, key_str, value, &required_size); // Use key_str
    if (err == ESP_OK) {
        ESP_LOGI(TAG, "Read timestamp key '%s': %lld", key_str, (long long)*value);
    } else if (err == ESP_ERR_NVS_NOT_FOUND) {
        ESP_LOGW(TAG, "Timestamp key '%s' not found", key_str);
        //*value = 0; // Default value if key not found
    } else {
        ESP_LOGE(TAG, "Failed to read timestamp key '%s': %s", key_str, esp_err_to_name(err));
    }

    nvs_close(handle);
    return err;
}

// Funzione per scrivere un valore string nella NVS
esp_err_t nvs_manager_set_string(uint64_t key, const char *value) {
    char key_str[22];
    snprintf(key_str, sizeof(key_str), "%llu", key); // Convert key to string

    char current_value[128]; // Buffer temporaneo per confrontare la stringa
    esp_err_t err = nvs_manager_get_string(key, current_value, sizeof(current_value));
    if (err == ESP_OK && strcmp(current_value, value) == 0) {
        ESP_LOGI(TAG, "Key '%s' already has the value '%s', skipping write", key_str, value);
        return ESP_OK; // Evita la scrittura se il valore è già uguale
    }

    nvs_handle_t handle;
    err = nvs_open_from_partition(NVS_PARTITION_NAME, NVS_NAMESPACE, NVS_READWRITE, &handle);
    if (err != ESP_OK) {
        //ESP_LOGE(TAG, "Failed to open NVS: %s", esp_err_to_name(err));
        return err;
    }

    err = nvs_set_str(handle, key_str, value); // Use key_str
    if (err == ESP_OK) {
        ESP_LOGI(TAG, "Saved string key '%s': %s", key_str, value);
        nvs_commit(handle);
    } else {
        ESP_LOGE(TAG, "Failed to save string key '%s': %s", key_str, esp_err_to_name(err));
    }

    nvs_close(handle);
    return err;
}

// Funzione per leggere un valore string dalla NVS
esp_err_t nvs_manager_get_string(uint64_t key, char *value, size_t max_len) {
    char key_str[22];
    snprintf(key_str, sizeof(key_str), "%llu", key); // Convert key to string

    nvs_handle_t handle;
    esp_err_t err = nvs_open_from_partition(NVS_PARTITION_NAME, NVS_NAMESPACE, NVS_READONLY, &handle);
    if (err != ESP_OK) {
        //ESP_LOGE(TAG, "Failed to open NVS: %s", esp_err_to_name(err));
        return err;
    }

    size_t required_size;
    err = nvs_get_str(handle, key_str, NULL, &required_size); // Use key_str
    if (err == ESP_OK) {
        if (required_size > max_len) {
            ESP_LOGE(TAG, "Buffer too small for key '%s'", key_str);
            nvs_close(handle);
            return ESP_ERR_NVS_INVALID_LENGTH;
        }
        err = nvs_get_str(handle, key_str, value, &required_size);
        if (err == ESP_OK) {
            ESP_LOGI(TAG, "Read string key '%s': %s", key_str, value);
        } else {
            ESP_LOGE(TAG, "Failed to read string key '%s': %s", key_str, esp_err_to_name(err));
        }
    } else if (err == ESP_ERR_NVS_NOT_FOUND) {
        ESP_LOGW(TAG, "String key '%s' not found", key_str);
        if (max_len > 0) {
            value[0] = '\0'; // Default empty string if key not found
        }
    } else {
        ESP_LOGE(TAG, "Failed to read string key '%s': %s", key_str, esp_err_to_name(err));
    }

    nvs_close(handle);
    return err;
}

// Funzione per cancellare tutti i dati nella NVS
esp_err_t nvs_manager_erase_all(void) {
    nvs_handle_t handle;
    esp_err_t err = nvs_open_from_partition(NVS_PARTITION_NAME, NVS_NAMESPACE, NVS_READWRITE, &handle);
    if (err != ESP_OK) {
        //ESP_LOGE(TAG, "Failed to open NVS: %s", esp_err_to_name(err));
        return err;
    }

    err = nvs_erase_all(handle);
    if (err == ESP_OK) {
        ESP_LOGI(TAG, "All keys erased successfully");
        nvs_commit(handle);
    } else {
        ESP_LOGE(TAG, "Failed to erase keys: %s", esp_err_to_name(err));
    }

    nvs_close(handle);
    return err;
}