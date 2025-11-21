#include "hmi_tools.h"
#include "../../HMI.h"
#include "../../services/mqtt/mqtt.h"
#include <math.h>
#include "../../services/NVS/nvs_manager.h"
#include <nvs_flash.h>
#include <nvs.h>

#define _SET 1
#define _RESET 2

#ifndef ARRAY_SIZE
#define ARRAY_SIZE(arr) (sizeof(arr) / sizeof((arr)[0]))
#endif

static size_t HMI_array_length = ARRAY_SIZE(id);

void PLC_to_NVS() {
  esp_err_t write;
    // Iterate through the PLC_pointer array
    for (size_t i = 0; i < HMI_array_length; i++) {
        uint64_t key = id[i]; // Use the id corresponding to the index
        switch (type[i]) {
            case REAL: {
                float valueStored = *(float *)PLC_pointer[i];
                ESP_LOGI("DEBUG", "Writing tag number %llu value to nvs: %f", id[i], valueStored);
                write = nvs_manager_set_float(key, valueStored);
                if (write != ESP_OK) {
                    ESP_LOGE("DEBUG", "Error writing to NVS: %s", esp_err_to_name(write));
                    continue;
                }
                break;
            }
            case INT: {
                int valueStored = *(int *)PLC_pointer[i];
                ESP_LOGI("DEBUG", "Writing tag number %llu value to nvs: %d", id[i], valueStored);
                write = nvs_manager_set_int(key, valueStored);
                if (write != ESP_OK) {
                    ESP_LOGE("DEBUG", "Error writing to NVS: %s", esp_err_to_name(write));
                    continue;
                }
                break;
            }
            case BOOL: {
                bool valueStored = *(bool *)PLC_pointer[i];
                ESP_LOGI("DEBUG", "Writing tag number %llu value to nvs: %d", id[i], valueStored);
                write = nvs_manager_set_bool(key, valueStored);
                if (write != ESP_OK) {
                    ESP_LOGE("DEBUG", "Error writing to NVS: %s", esp_err_to_name(write));
                    continue;
                }
                break;
            }
            default:
                ESP_LOGW("DEBUG", "Unhandled type '%d' for key '%llu'", type[i], key);
                break;
        }
    }
}

void NVS_to_PLC () {
  esp_err_t read;
  esp_err_t write;
      // Iterate through the PLC_pointer array
      for (size_t i = 0; i < HMI_array_length; i++) {
        uint64_t key = id[i]; // Use the id corresponding to the index
        switch (type[i]) {
            case REAL: {
                float valueStored = 0.0f; // Initialize the variable to store the value
                read = nvs_manager_get_float(key, &valueStored);
                ESP_LOGI("DEBUG", "Read tag number %llu value from nvs: %f", id[i], valueStored);
                if (read == ESP_ERR_NVS_NOT_FOUND) {
                    ESP_LOGW("DEBUG", "Float key '%llu' not found, initializing with default value", key);
                    // Write the initial value to NVS
                    write = nvs_manager_set_float(key, *(float *)PLC_pointer[i]);
                    if (write != ESP_OK) {
                        ESP_LOGE("DEBUG", "Error writing default value to NVS: %s", esp_err_to_name(write));
                        continue;
                    }
                } else if (read != ESP_OK) {
                    ESP_LOGE("DEBUG", "Error reading from NVS: %s", esp_err_to_name(read));
                    continue;
                } else {
                    // If the value is successfully read, assign it to the current pointer
                    *(float *)PLC_pointer[i] = valueStored;
                    ESP_LOGI("DEBUG", "Read tag number %llu value from nvs: %f", id[i], valueStored);
                }
                break;
            }
            case INT: {
                int valueStored = 0; // Initialize the variable to store the value
                read = nvs_manager_get_int(key, &valueStored);
                ESP_LOGI("DEBUG", "Read tag number %llu value from nvs: %d", id[i], valueStored);
                if (read == ESP_ERR_NVS_NOT_FOUND) {
                    ESP_LOGW("DEBUG", "Int key '%llu' not found, initializing with default value", key);
                    // Write the initial value to NVS
                    write = nvs_manager_set_int(key, *(int *)PLC_pointer[i]);
                    if (write != ESP_OK) {
                        ESP_LOGE("DEBUG", "Error writing default value to NVS: %s", esp_err_to_name(write));
                        continue;
                    }
                } else if (read != ESP_OK) {
                    ESP_LOGE("DEBUG", "Error reading from NVS: %s", esp_err_to_name(read));
                    continue;
                } else {
                    // If the value is successfully read, assign it to the current pointer
                    *(int *)PLC_pointer[i] = valueStored;
                    ESP_LOGI("DEBUG", "Read tag number %llu value from nvs: %d", id[i], valueStored);
                }
                break;
            }
            case BOOL: {
                bool valueStored = false; // Initialize the variable to store the value
                read = nvs_manager_get_bool(key, &valueStored);
                if (read == ESP_ERR_NVS_NOT_FOUND) {
                    ESP_LOGW("DEBUG", "Bool key '%llu' not found, initializing with default value", key);
                    // Write the initial value to NVS
                    write = nvs_manager_set_bool(key, *(bool *)PLC_pointer[i]);
                    if (write != ESP_OK) {
                        ESP_LOGE("DEBUG", "Error writing default value to NVS: %s", esp_err_to_name(write));
                        continue;
                    }
                } else if (read != ESP_OK) {
                    ESP_LOGE("DEBUG", "Error reading from NVS: %s", esp_err_to_name(read));
                    continue;
                } else {
                    // If the value is successfully read, assign it to the current pointer
                    *(bool *)PLC_pointer[i] = valueStored;
                    ESP_LOGI("DEBUG", "Read tag number %llu value from nvs: %d", id[i], valueStored);
                }
                break;
            }
            default:
                ESP_LOGW("DEBUG", "Unhandled type '%d' for key '%llu'", type[i], key);
                break;
        }
    }
}


//Initialization of the retentive tags in the nvs
void sclib_init() {
    esp_err_t read;
    esp_err_t write;

    // Initialize the NVS at the very first startup
    int init = 0;
    read = nvs_manager_get_int(0, &init);
    if (read == ESP_ERR_NVS_NOT_FOUND) {
        ESP_LOGW("DEBUG", "Init key not found, initializing NVS");
        write = nvs_manager_set_int(0, 1);
        if (write != ESP_OK) {
            ESP_LOGE("DEBUG", "Error writing default value to NVS: %s", esp_err_to_name(write));
            return;
        }
        ESP_LOGI("DEBUG", "NVS initialization, moving PLC values to NVS");
        PLC_to_NVS();
    } else if (read != ESP_OK) {
        ESP_LOGE("DEBUG", "Error reading from NVS: %s", esp_err_to_name(read));
    } else {
        ESP_LOGI("DEBUG", "NVS already initialized, skipping initialization and moving values to PLC");
        NVS_to_PLC();
    }
}

void sclib_logic (LogicSelection *logic_selection) {

  if (logic_selection->Status == 0) {
    int status = 1;
    for (size_t i = 0; i < HMI_array_length; i++) {
      if (PLC_pointer[i] == (void *)&logic_selection->Status) {
        uint64_t key = id[i]; // Use the id corresponding to the index
        esp_err_t write = nvs_manager_set_int(key, status);
        if (write != ESP_OK) {
          //ESP_LOGE("DEBUG", "Error writing to NVS: %s", esp_err_to_name(write));
          return;
        }
        break;
      }
    }
    logic_selection->Status = status;
    ESP_LOGI("DEBUG", "Status initialization: %d", logic_selection->Status);
  }
  //management
  for (int i = 0; i < 8; i++) {
    if (1 & (logic_selection->Command >> i)) {
      ESP_LOGI("DEBUG", "Received Value: %d", logic_selection->Command);
      for (size_t i = 0; i < HMI_array_length; i++) {
        if (PLC_pointer[i] == (void *)&logic_selection->Status) {
          uint64_t key = id[i]; // Use the id corresponding to the index
          esp_err_t write = nvs_manager_set_int(key, logic_selection->Command);
          if (write != ESP_OK) {
            //ESP_LOGE("DEBUG", "Error writing to NVS: %s", esp_err_to_name(write));
            return;
          }
          break;
        }
      }
      logic_selection->Status = logic_selection->Command;
      int command = 0;
      logic_selection->Command = command;
      break;
    }
  }
}

void sclib_logic_SR (LogicSelection *logic_selection, int resetNotAllowed, int resetForced, int setNotAllowed, int setForced) {
  //initialization
  if (logic_selection->Status == 0){
    int status = 1;
    logic_selection->Status = status;
  }
  //management
  int command = 0;
  if (resetForced){
    logic_selection->Status = _RESET;
  }
  else if (setForced && !resetForced){
    logic_selection->Status = _SET;
  }
  else {
    switch (logic_selection->Command) {
      case _SET:
        if (!setNotAllowed) {
          if (logic_selection->Status != logic_selection->Command) {
            logic_selection->Status = logic_selection->Command;
          }
        }
        logic_selection->Command = command;
        break;
      case _RESET:
        if (!resetNotAllowed) {
          if (logic_selection->Status != logic_selection->Command) {
            logic_selection->Status = logic_selection->Command;
          }
        }
        logic_selection->Command = command;
        break;
      default:
        // Handle invalid command if necessary
        break;
    }
  }
}

void sclib_logic_2 (LogicSelection *logic_selection, int _1NotAllowed, int _1Forced, int _2NotAllowed, int _2Forced) {
  //initialization
  if (logic_selection->Status == 0){
    int status = 1;
    logic_selection->Status = status;
  }
  //management
  int command = 0;
  if (_1Forced){
    logic_selection->Status = 1;
  }
  else if (_2Forced && !_1Forced){
    logic_selection->Status = 2;
  }
  else {
    switch (logic_selection->Command) {
      case 1:
        if (!_1NotAllowed) {
          if (logic_selection->Status != logic_selection->Command) {
            logic_selection->Status = logic_selection->Command;
          }
        }
        logic_selection->Command = command;
        break;
      case 2:
        if (!_2NotAllowed) {
          if (logic_selection->Status != logic_selection->Command) {
            logic_selection->Status = logic_selection->Command;
          }
        }
        logic_selection->Command = command;
        break;
      default:
        // Handle invalid command if necessary
        break;
    }
  }
}

void sclib_logic_3 (LogicSelection *logic_selection, int _1NotAllowed, int _1Forced, int _2NotAllowed, int _2Forced, int _3NotAllowed, int _3Forced) {
  //initialization
  if (logic_selection->Status == 0){
    int status = 1;
    logic_selection->Status = status;
  }
  //management
  int command = 0;
  if (_1Forced){
    logic_selection->Status = 1;
  }
  else if (_2Forced && !_1Forced){
    logic_selection->Status = 2;
  }
  else if (_3Forced && !_1Forced && !_2Forced){
    logic_selection->Status = 4;
  }
  else {
    switch (logic_selection->Command) {
      case 1:
        if (!_1NotAllowed) {
          if (logic_selection->Status != logic_selection->Command) {
            logic_selection->Status = logic_selection->Command;
          }
        }
        logic_selection->Command = command;
        break;
      case 2:
        if (!_2NotAllowed) {
          if (logic_selection->Status != logic_selection->Command) {
            logic_selection->Status = logic_selection->Command;
          }
        }
        logic_selection->Command = command;
        break;
      case 4:
        if (!_3NotAllowed) {
          if (logic_selection->Status != logic_selection->Command) {
            logic_selection->Status = logic_selection->Command;
          }
        }
        logic_selection->Command = command;
        break;
      default:
        // Handle invalid command if necessary
        break;
    }
  }
}

void sclib_logic_4 (LogicSelection *logic_selection, int _1NotAllowed, int _1Forced, int _2NotAllowed, int _2Forced, int _3NotAllowed, int _3Forced, int _4NotAllowed, int _4Forced) {
  //initialization
  if (logic_selection->Status == 0){
    int status = 1;
    logic_selection->Status = status;
  }
  //management
  int command = 0;
  if (_1Forced){
    logic_selection->Status = 1;
  }
  else if (_2Forced && !_1Forced){
    logic_selection->Status = 2;
  }
  else if (_3Forced && !_1Forced && !_2Forced){
    logic_selection->Status = 4;
  }
  else if (_4Forced && !_1Forced && !_2Forced && !_3Forced){
    logic_selection->Status = 8;
  }
  else {
    switch (logic_selection->Command) {
      case 1:
        if (!_1NotAllowed) {
          if (logic_selection->Status != logic_selection->Command) {
            logic_selection->Status = logic_selection->Command;
          }
        }
        logic_selection->Command = command;
        break;
      case 2:
        if (!_2NotAllowed) {
          if (logic_selection->Status != logic_selection->Command) {
            logic_selection->Status = logic_selection->Command;
          }
        }
        logic_selection->Command = command;
        break;
      case 4:
        if (!_3NotAllowed) {
          if (logic_selection->Status != logic_selection->Command) {
            logic_selection->Status = logic_selection->Command;
          }
        }
        logic_selection->Command = command;
        break;
      case 8:
        if (!_4NotAllowed) {
          if (logic_selection->Status != logic_selection->Command) {
            logic_selection->Status = logic_selection->Command;
          }
        }
        logic_selection->Command = command;
        break;
      default:
        // Handle invalid command if necessary
        break;
    }
  }
}

void sclib_logic_5 (LogicSelection *logic_selection, int _1NotAllowed, int _1Forced, int _2NotAllowed, int _2Forced, int _3NotAllowed, int _3Forced, int _4NotAllowed, int _4Forced, int _5NotAllowed, int _5Forced) {
  //initialization
  if (logic_selection->Status == 0){
    int status = 1;
    logic_selection->Status = status;
  }
  //management
  int command = 0;
  if (_1Forced){
    logic_selection->Status = 1;
  }
  else if (_2Forced && !_1Forced){
    logic_selection->Status = 2;
  }
  else if (_3Forced && !_1Forced && !_2Forced){
    logic_selection->Status = 4;
  }
  else if (_4Forced && !_1Forced && !_2Forced && !_3Forced){
    logic_selection->Status = 8;
  }
  else if (_5Forced && !_1Forced && !_2Forced && !_3Forced && !_4Forced){
    logic_selection->Status = 16;
  }
  else {
    switch (logic_selection->Command) {
      case 1:
        if (!_1NotAllowed) {
          if (logic_selection->Status != logic_selection->Command) {
            logic_selection->Status = logic_selection->Command;
          }
        }
        logic_selection->Command = command;
        break;
      case 2:
        if (!_2NotAllowed) {
          if (logic_selection->Status != logic_selection->Command) {
            logic_selection->Status = logic_selection->Command;
          }
        }
        logic_selection->Command = command;
        break;
      case 4:
        if (!_3NotAllowed) {
          if (logic_selection->Status != logic_selection->Command) {
            logic_selection->Status = logic_selection->Command;
          }
        }
        logic_selection->Command = command;
        break;
      case 8:
        if (!_4NotAllowed) {
          if (logic_selection->Status != logic_selection->Command) {
            logic_selection->Status = logic_selection->Command;
          }
        }
        logic_selection->Command = command;
        break;
      case 16:
        if (!_5NotAllowed) {
          if (logic_selection->Status != logic_selection->Command) {
            logic_selection->Status = logic_selection->Command;
          }
        }
        logic_selection->Command = command;
        break;
      default:
        // Handle invalid command if necessary
        break;
    }
  }
}

void sclib_logic_6 (LogicSelection *logic_selection, int _1NotAllowed, int _1Forced, int _2NotAllowed, int _2Forced, int _3NotAllowed, int _3Forced, int _4NotAllowed, int _4Forced, int _5NotAllowed, int _5Forced, int _6NotAllowed, int _6Forced) {
  //initialization
  if (logic_selection->Status == 0){
    int status = 1;
    logic_selection->Status = status;
  }
  //management
  int command = 0;
  if (_1Forced){
    logic_selection->Status = 1;
  }
  else if (_2Forced && !_1Forced){
    logic_selection->Status = 2;
  }
  else if (_3Forced && !_1Forced && !_2Forced){
    logic_selection->Status = 4;
  }
  else if (_4Forced && !_1Forced && !_2Forced && !_3Forced){
    logic_selection->Status = 8;
  }
  else if (_5Forced && !_1Forced && !_2Forced && !_3Forced && !_4Forced){
    logic_selection->Status = 16;
  }
  else if (_6Forced && !_1Forced && !_2Forced && !_3Forced && !_4Forced && !_5Forced){
    logic_selection->Status = 32;
  }
  else {
    switch (logic_selection->Command) {
      case 1:
        if (!_1NotAllowed) {
          if (logic_selection->Status != logic_selection->Command) {
            logic_selection->Status = logic_selection->Command;
          }
        }
        logic_selection->Command = command;
        break;
      case 2:
        if (!_2NotAllowed) {
          if (logic_selection->Status != logic_selection->Command) {
            logic_selection->Status = logic_selection->Command;
          }
        }
        logic_selection->Command = command;
        break;
      case 4:
        if (!_3NotAllowed) {
          if (logic_selection->Status != logic_selection->Command) {
            logic_selection->Status = logic_selection->Command;
          }
        }
        logic_selection->Command = command;
        break;
      case 8:
        if (!_4NotAllowed) {
          if (logic_selection->Status != logic_selection->Command) {
            logic_selection->Status = logic_selection->Command;
          }
        }
        logic_selection->Command = command;
        break;
      case 16:
        if (!_5NotAllowed) {
          if (logic_selection->Status != logic_selection->Command) {
            logic_selection->Status = logic_selection->Command;
          }
        }
        logic_selection->Command = command;
        break;
      case 32:
        if (!_6NotAllowed) {
          if (logic_selection->Status != logic_selection->Command) {
            logic_selection->Status = logic_selection->Command;
          }
        }
        logic_selection->Command = command;
        break;
      default:
        // Handle invalid command if necessary
        break;
    }
  }
}

void sclib_logic_7 (LogicSelection *logic_selection, int _1NotAllowed, int _1Forced, int _2NotAllowed, int _2Forced, int _3NotAllowed, int _3Forced, int _4NotAllowed, int _4Forced, int _5NotAllowed, int _5Forced, int _6NotAllowed, int _6Forced, int _7NotAllowed, int _7Forced) {
  //initialization
  if (logic_selection->Status == 0){
    int status = 1;
    logic_selection->Status = status;
  }
  //management
  int command = 0;
  if (_1Forced){
    logic_selection->Status = 1;
  }
  else if (_2Forced && !_1Forced){
    logic_selection->Status = 2;
  }
  else if (_3Forced && !_1Forced && !_2Forced){
    logic_selection->Status = 4;
  }
  else if (_4Forced && !_1Forced && !_2Forced && !_3Forced){
    logic_selection->Status = 8;
  }
  else if (_5Forced && !_1Forced && !_2Forced && !_3Forced && !_4Forced){
    logic_selection->Status = 16;
  }
  else if (_6Forced && !_1Forced && !_2Forced && !_3Forced && !_4Forced && !_5Forced){
    logic_selection->Status = 32;
  }
  else if (_7Forced && !_1Forced && !_2Forced && !_3Forced && !_4Forced && !_5Forced && !_6Forced){
    logic_selection->Status = 64;
  }
  else {
    switch (logic_selection->Command) {
      case 1:
        if (!_1NotAllowed) {
          if (logic_selection->Status != logic_selection->Command) {
            logic_selection->Status = logic_selection->Command;
          }
        }
        logic_selection->Command = command;
        break;
      case 2:
        if (!_2NotAllowed) {
          if (logic_selection->Status != logic_selection->Command) {
            logic_selection->Status = logic_selection->Command;
          }
        }
        logic_selection->Command = command;
        break;
      case 4:
        if (!_3NotAllowed) {
          if (logic_selection->Status != logic_selection->Command) {
            logic_selection->Status = logic_selection->Command;
          }
        }
        logic_selection->Command = command;
        break;
      case 8:
        if (!_4NotAllowed) {
          if (logic_selection->Status != logic_selection->Command) {
            logic_selection->Status = logic_selection->Command;
          }
        }
        logic_selection->Command = command;
        break;
      case 16:
        if (!_5NotAllowed) {
          if (logic_selection->Status != logic_selection->Command) {
            logic_selection->Status = logic_selection->Command;
          }
        }
        logic_selection->Command = command;
        break;
      case 32:
        if (!_6NotAllowed) {
          if (logic_selection->Status != logic_selection->Command) {
            logic_selection->Status = logic_selection->Command;
          }
        }
        logic_selection->Command = command;
        break;
      case 64:
        if (!_7NotAllowed) {
          if (logic_selection->Status != logic_selection->Command) {
            logic_selection->Status = logic_selection->Command;
          }
        }
        logic_selection->Command = command;
        break;
      default:
        // Handle invalid command if necessary
        break;
    }
  }
}

void sclib_logic_8 (LogicSelection *logic_selection, int _1NotAllowed, int _1Forced, int _2NotAllowed, int _2Forced, int _3NotAllowed, int _3Forced, int _4NotAllowed, int _4Forced, int _5NotAllowed, int _5Forced, int _6NotAllowed, int _6Forced, int _7NotAllowed, int _7Forced, int _8NotAllowed, int _8Forced) {
  //initialization
  if (logic_selection->Status == 0){
    int status = 1;
    logic_selection->Status = status;
  }
  //management
  int command = 0;
  if (_1Forced){
    logic_selection->Status = 1;
  }
  else if (_2Forced && !_1Forced){
    logic_selection->Status = 2;
  }
  else if (_3Forced && !_1Forced && !_2Forced){
    logic_selection->Status = 4;
  }
  else if (_4Forced && !_1Forced && !_2Forced && !_3Forced){
    logic_selection->Status = 8;
  }
  else if (_5Forced && !_1Forced && !_2Forced && !_3Forced && !_4Forced){
    logic_selection->Status = 16;
  }
  else if (_6Forced && !_1Forced && !_2Forced && !_3Forced && !_4Forced && !_5Forced){
    logic_selection->Status = 32;
  }
  else if (_7Forced && !_1Forced && !_2Forced && !_3Forced && !_4Forced && !_5Forced && !_6Forced){
    logic_selection->Status = 64;
  }
  else if (_8Forced && !_1Forced && !_2Forced && !_3Forced && !_4Forced && !_5Forced && !_6Forced && !_7Forced){
    logic_selection->Status = 128;
  }
  else {
    switch (logic_selection->Command) {
      case 1:
        if (!_1NotAllowed) {
          if (logic_selection->Status != logic_selection->Command) {
            logic_selection->Status = logic_selection->Command;
          }
        }
        logic_selection->Command = command;
        break;
      case 2:
        if (!_2NotAllowed) {
          if (logic_selection->Status != logic_selection->Command) {
            logic_selection->Status = logic_selection->Command;
          }
        }
        logic_selection->Command = command;
        break;
      case 4:
        if (!_3NotAllowed) {
          if (logic_selection->Status != logic_selection->Command) {
            logic_selection->Status = logic_selection->Command;
          }
        }
        logic_selection->Command = command;
        break;
      case 8:
        if (!_4NotAllowed) {
          if (logic_selection->Status != logic_selection->Command) {
            logic_selection->Status = logic_selection->Command;
          }
        }
        logic_selection->Command = command;
        break;
      case 16:
        if (!_5NotAllowed) {
          if (logic_selection->Status != logic_selection->Command) {
            logic_selection->Status = logic_selection->Command;
          }
        }
        logic_selection->Command = command;
        break;
      case 32:
        if (!_6NotAllowed) {
          if (logic_selection->Status != logic_selection->Command) {
            logic_selection->Status = logic_selection->Command;
          }
        }
        logic_selection->Command = command;
        break;
      case 64:
        if (!_7NotAllowed) {
          if (logic_selection->Status != logic_selection->Command) {
            logic_selection->Status = logic_selection->Command;
          }
        }
        logic_selection->Command = command;
        break;
      case 128:
        if (!_8NotAllowed) {
          if (logic_selection->Status != logic_selection->Command) {
            logic_selection->Status = logic_selection->Command;
          }
        }
        logic_selection->Command = command;
        break;
      default:
        // Handle invalid command if necessary
        break;
    }
  }
}

// Funzione per ottenere il valore dell'ennesimo bit di un intero
int get_bit_value(uint8_t number, int bit_position) {
  return (number >> bit_position) & 1;
}

// Funzione generica per gestire la logic selection
void sclib_logic_generic(LogicSelection *logic_selection, uint8_t *force, uint8_t *NotAllowed) {
  //initialization
  if (logic_selection->Status == 0){
    int status = 1;
    logic_selection->Status = status;
  }
  //management
  int forced = 0;
  for (int i = 0; i < 8; i++) {
    int force_bit = get_bit_value(*force, i);
    if (force_bit) {
      if (logic_selection->Status != 1 << i) {
        int val = 1 << i;
        logic_selection->Status = val;
      }
      forced = 1;
      break;
    }
  }
  if (!forced) {
    for (int i = 0; i < 8; i++) {
      int NotAllowed_bit = get_bit_value(*NotAllowed, i);
      if (1 & (logic_selection->Command >> i)) {
        if (!NotAllowed_bit) {
          logic_selection->Status = logic_selection->Command;
          int command = 0;
          logic_selection->Command = command;
          break;
        }
      }
    }
  }
}



// Funzione generica per gestire un Set
void sclib_Set(Set *set, int force, float forceValue, int NotAllowed) {

  // First initialization
  if (!set->Init) {
    set->Set.InputValue = set->Set.Value;
    set->Init = 1; // Mark as initialized
    ESP_LOGI("DEBUG", "Set initialization: %f", set->Set.Value);
  }

  //Min and Max check
  if (set->Limit.Min >= set->Limit.Max) {
    return;
  }

  if (set->Set.Value > set->Limit.Max){
    uint64_t key = -1;
    for (size_t i = 0; i < HMI_array_length; i++) {
      if (PLC_pointer[i] == (void *)&set->Set.Value) {
      key = id[i]; // Use the id corresponding to the index
      break;
      }
    }
    if (key != -1) {
      esp_err_t write = nvs_manager_set_float(key, set->Limit.Max);
      if (write != ESP_OK) {
        ESP_LOGE("DEBUG", "Error writing to NVS: %s", esp_err_to_name(write));
        return;
      }
    }
    set->Set.Value = set->Limit.Max;
    set->Set.InputValue = set->Limit.Max;
  }
  if (set->Set.Value < set->Limit.Min) {
    uint64_t key = -1;
    for (size_t i = 0; i < HMI_array_length; i++) {
      if (PLC_pointer[i] == (void *)&set->Set.Value) {
        key = id[i]; // Use the id corresponding to the index
        break;
      }
    }
    if (key != -1) {
      esp_err_t write = nvs_manager_set_float(key, set->Limit.Min);
      if (write != ESP_OK) {
        ESP_LOGE("DEBUG", "Error writing to NVS: %s", esp_err_to_name(write));
        return;
      }
    }
    set->Set.Value = set->Limit.Min;
    set->Set.InputValue = set->Limit.Min;
  } 
  
  //Forced value
  if (force) {
    if (forceValue != set->Set.Value) {
      if (forceValue <= set->Limit.Max && forceValue >= set->Limit.Min) {
        uint64_t key = -1;
        for (size_t i = 0; i < HMI_array_length; i++) {
          if (PLC_pointer[i] == (void *)&set->Set.Value) {
          key = id[i]; // Use the id corresponding to the index
          break;
          }
        }
        if (key != -1) {
          esp_err_t write = nvs_manager_set_float(key, forceValue);
          if (write != ESP_OK) {
            ESP_LOGE("DEBUG", "Error writing to NVS: %s", esp_err_to_name(write));
            return;
          }
        }
        set->Set.Value = forceValue;
        set->Set.InputValue = forceValue;
      }
    }
  } else {
    if (set->Set.InputValue != set->Set.Value) {
      // Check if the value inputing is allowed
      if (NotAllowed){
        set->Set.InputValue = set->Set.Value;
      } else {
        // If allowed, check if the value is in the range
        if (set->Set.InputValue <= set->Limit.Max && set->Set.InputValue >= set->Limit.Min) {
          // If is in range, update the value
          uint64_t key = -1;
          for (size_t i = 0; i < HMI_array_length; i++) {
            if (PLC_pointer[i] == (void *)&set->Set.Value) {
            key = id[i]; // Use the id corresponding to the index
            break;
            }
          }
          if (key != -1) {
            esp_err_t write = nvs_manager_set_float(key, set->Set.InputValue);
            if (write != ESP_OK) {
              ESP_LOGE("DEBUG", "Error writing to NVS: %s", esp_err_to_name(write));
              return;
            }
          }
          set->Set.Value = set->Set.InputValue;
        } else {
          // If is not in range, update the input value with the current value
          set->Set.InputValue = set->Set.Value;
        }
      }
    }
  }
}


// Funzione generica per gestire un SetAct
void sclib_SetAct(SetAct *setact, int force, float forceValue, int NotAllowed) {

  // First initialization
  if (!setact->Init) {
    setact->Set.InputValue = setact->Set.Value;
    setact->Init = 1; // Mark as initialized
    ESP_LOGI("DEBUG", "SetAct Set initialization: %f", setact->Set.Value);
  }

  //Min and Max check
  if (setact->Limit.Min >= setact->Limit.Max) {
    return;
  }

  if (setact->Set.Value > setact->Limit.Max){
    uint64_t key = -1;
    for (size_t i = 0; i < HMI_array_length; i++) {
      if (PLC_pointer[i] == (void *)&setact->Set.Value) {
      key = id[i]; // Use the id corresponding to the index
      break;
      }
    }
    if (key != -1) {
      esp_err_t write = nvs_manager_set_float(key, setact->Limit.Max);
      if (write != ESP_OK) {
        ESP_LOGE("DEBUG", "Error writing to NVS: %s", esp_err_to_name(write));
        return;
      }
    }
    setact->Set.Value = setact->Limit.Max;
    setact->Set.InputValue = setact->Limit.Max;
  }
  if (setact->Set.Value < setact->Limit.Min) {
    uint64_t key = -1;
    for (size_t i = 0; i < HMI_array_length; i++) {
      if (PLC_pointer[i] == (void *)&setact->Set.Value) {
        key = id[i]; // Use the id corresponding to the index
        break;
      }
    }
    if (key != -1) {
      esp_err_t write = nvs_manager_set_float(key, setact->Limit.Min);
      if (write != ESP_OK) {
        ESP_LOGE("DEBUG", "Error writing to NVS: %s", esp_err_to_name(write));
        return;
      }
    }
    setact->Set.Value = setact->Limit.Min;
    setact->Set.InputValue = setact->Limit.Min;
  } 
  
  //Forced value
  if (force) {
    if (forceValue != setact->Set.Value) {
      if (forceValue <= setact->Limit.Max && forceValue >= setact->Limit.Min) {
        uint64_t key = -1;
        for (size_t i = 0; i < HMI_array_length; i++) {
          if (PLC_pointer[i] == (void *)&setact->Set.Value) {
          key = id[i]; // Use the id corresponding to the index
          break;
          }
        }
        if (key != -1) {
          esp_err_t write = nvs_manager_set_float(key, forceValue);
          if (write != ESP_OK) {
            ESP_LOGE("DEBUG", "Error writing to NVS: %s", esp_err_to_name(write));
            return;
          }
        }
        setact->Set.Value = forceValue;
        setact->Set.InputValue = forceValue;
      }
    }
  } else {
    if (setact->Set.InputValue != setact->Set.Value) {
      // Check if the value inputing is allowed
      if (NotAllowed){
        setact->Set.InputValue = setact->Set.Value;
      } else {
        // If allowed, check if the value is in the range
        if (setact->Set.InputValue <= setact->Limit.Max && setact->Set.InputValue >= setact->Limit.Min) {
          // If is in range, update the value
          uint64_t key = -1;
          for (size_t i = 0; i < HMI_array_length; i++) {
            if (PLC_pointer[i] == (void *)&setact->Set.Value) {
            key = id[i]; // Use the id corresponding to the index
            break;
            }
          }
          if (key != -1) {
            esp_err_t write = nvs_manager_set_float(key, setact->Set.InputValue);
            if (write != ESP_OK) {
              ESP_LOGE("DEBUG", "Error writing to NVS: %s", esp_err_to_name(write));
              return;
            }
          }
          setact->Set.Value = setact->Set.InputValue;
        } else {
          // If is not in range, update the input value with the current value
          setact->Set.InputValue = setact->Set.Value;
        }
      }
    }
  }
}

// Funzione generica per scrivere un valore nell'act in Act
void sclib_writeAct(Act *act, float value){  
  int scale = (int)pow(10, act->Decimals);
  act->Act.HMIValue = (float)((int)(value * scale + 0.5)) / scale; // Usa floor per evitare imprecisioni
  act->Act.Value = value;

  // Trova l'indice di act->Value in PLC_pointer
  for (size_t i = 0; i < HMI_array_length; i++) {
    if (PLC_pointer[i] == (void *)&act->Act.Value) {
      // Scrivi il valore di percentage nel corrispondente HMI_pointer
      *(float *)HMI_pointer[i] = value;
      break;
    }
  }
};

// Funzione generica per scrivere un valore nell'act in SetAct
void sclib_writeSetAct(SetAct *setact, float value){  
  int scale = (int)pow(10, setact->Decimals);
  setact->Act.HMIValue = (float)((int)(value * scale + 0.5)) / scale; // Usa floor per evitare imprecisioni
  setact->Act.Value = value;

  // Per evitare che il valore venga rilevato come differente dalla funzione che si occupa di pubblicare il refresh, il value viene copiato identico sulla sua controparte HMI.
  // Questo perché non voglio che venga trasmesso al broker a causa dell'elevato rate di refresh e della scarsa rilevanza su HMI
  // Trova l'indice di setact->Value in PLC_pointer
  for (size_t i = 0; i < HMI_array_length; i++) {
    if (PLC_pointer[i] == (void *)&setact->Act.Value) {
      // Scrivi il valore di percentage nel corrispondente HMI_pointer
      *(float *)HMI_pointer[i] = value;
      break;
    }
  }
};