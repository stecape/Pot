/*
 * SPDX-FileCopyrightText: 2010-2022 Espressif Systems (Shanghai) CO LTD
 *
 * SPDX-License-Identifier: CC0-1.0
 */

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/gptimer.h"
#include "esp_log.h"
#include "esp_event.h"
#include "HMI.h"
#include "sclib/hmi_tools/hmi_tools.h"
#include "peripherials/led/led.h"
#include "services/mqtt/mqtt.h"
#include "services/Wifi/Wifi.h"
#include "services/NVS/nvs_manager.h"
#include "services/battery/battery.h"
#include "sclib/alarms/alarms.h"
#include "peripherials/sclibDS18B20/sclibDS18B20.h"
#include "peripherials/moisture/moisture.h"

void interrupt(void); // Prototipo per evitare implicit declaration

#define CONFIG_INTERRUPT_CYCLE_TIME_S 0.25

static TaskHandle_t task_handle = NULL;

void setup() {
  // Setup calls
  ESP_LOGI("HELLO", "Ciao Cacà, Ciao Tommy! =)");
  // Installa il servizio ISR GPIO una sola volta all'avvio (prima di qualsiasi setup periferica che usa interrupt)
  ESP_ERROR_CHECK(gpio_install_isr_service(0));
  // Create default event loop
  ESP_ERROR_CHECK(esp_event_loop_create_default());
  ESP_ERROR_CHECK(nvs_manager_init());
  Wifi_setup();
  mqtt_setup();
  sclib_init();
  led_setup();
  ds18b20_init();
  //battery_setup();
  moisture_init();
}


void loop() {
  // Loop calls
  led_loop();
  //battery_loop(&PLC.BatteryLevel);
  mqtt_updHMI(false);
  check_alarms();
  float temperature;
  ds18b20_read_temperature(&temperature);
  sclib_writeAct(&PLC.Temperature, temperature);
  sclib_Set(&PLC.Moisture.AnalogicMin, 0, 0, 0);
  sclib_Set(&PLC.Moisture.AnalogicMax, 0, 0, 0);
  sclib_SetAct(&PLC.Moisture.Moisture, 0, 0, 0);
  float moisture = read_moisture_level(PLC.Moisture.AnalogicMin.Set.Value, PLC.Moisture.AnalogicMax.Set.Value);
  sclib_writeSetAct(&PLC.Moisture.Moisture, moisture);
  float moistureAnalogic = read_moisture_analogic();
  sclib_writeAct(&PLC.Moisture.AnalogicRead, moistureAnalogic);
  PLC.Temperature.Limit.Min = -40;
  PLC.Temperature.Limit.Max = 50;
  alarm(&PLC.Faults.LowMoisture, PLC.Moisture.Moisture.Act.HMIValue<PLC.Moisture.Moisture.Set.Value, ALARM_REACTION_FAULT);

}



















// Definizione della ISR del timer
bool IRAM_ATTR timer_isr_callback(gptimer_handle_t timer, const gptimer_alarm_event_data_t *edata, void *user_ctx) {
    BaseType_t xHigherPriorityTaskWoken = pdFALSE;
    // Segnala il task
    vTaskNotifyGiveFromISR(task_handle, &xHigherPriorityTaskWoken);
    // Forza il contesto di switch se necessario
    return xHigherPriorityTaskWoken == pdTRUE;
}

// Funzione per configurare il timer
void init_timer() {
    gptimer_handle_t gptimer = NULL;
    gptimer_config_t timer_config = {
        .clk_src = GPTIMER_CLK_SRC_DEFAULT,
        .direction = GPTIMER_COUNT_UP,
        .resolution_hz = 1000000, // 1 MHz per ottenere microsecondi
    };
    gptimer_alarm_config_t alarm_config = {
        .alarm_count = CONFIG_INTERRUPT_CYCLE_TIME_S * 1000000, // 5 secondi (5 milioni di microsecondi)
        .reload_count = 0,
        .flags.auto_reload_on_alarm = true,
    };

    // Creazione e configurazione del timer
    ESP_ERROR_CHECK(gptimer_new_timer(&timer_config, &gptimer));
    ESP_ERROR_CHECK(gptimer_set_alarm_action(gptimer, &alarm_config));
    ESP_ERROR_CHECK(gptimer_register_event_callbacks(gptimer, &(gptimer_event_callbacks_t){
        .on_alarm = timer_isr_callback,
    }, NULL));
    ESP_ERROR_CHECK(gptimer_enable(gptimer));
    ESP_ERROR_CHECK(gptimer_start(gptimer));
}

// Task dedicato per gestire l'operazione richiesta
void interrupt_task(void *arg) {
  while (true) {
    // Attende la notifica dalla ISR
    ulTaskNotifyTake(pdTRUE, portMAX_DELAY);
    interrupt();
  }
}

void interrupt() {
  // Interrupt calls
}

void app_main(void) {
  // Inizializza in timer
  init_timer();
  // Creazione del task dedicato
  xTaskCreatePinnedToCore(interrupt_task, "interrupt_task", 4096, NULL, 10, &task_handle, 1);
  setup();
  while (true) {
    loop();
    vTaskDelay(10 / portTICK_PERIOD_MS);
  }
}