#include "moisture.h"
#include "driver/adc.h"
#include "driver/gpio.h"

int raw_moisture_value = 0;

void moisture_init() {
  gpio_config_t io_conf = {
    .pin_bit_mask = 1ULL << MOISTURE_SENSOR_PIN,
    .mode = GPIO_MODE_INPUT,
    .pull_up_en = GPIO_PULLUP_DISABLE,
    .pull_down_en = GPIO_PULLDOWN_DISABLE,
    .intr_type = GPIO_INTR_DISABLE
  };
  gpio_config(&io_conf);
}

float read_moisture_level(int min, int max) {
  int _min = min;
  int _max = max;
  if (_min >= _max) {
    _min = 0;
    _max = 100; // Default ADC range
  }
  // Valore a secco all'aria è 0, nel terriccio asciutto d'inverno è 950 analogica, appena umido 2800, bello zuppo sui 3100
  raw_moisture_value = adc1_get_raw(MOISTURE_SENSOR_ADC_CHANNEL); // Assuming GPIO39 (A3) is ADC1_CHANNEL_3
  float moisture_percentage = ((raw_moisture_value- _min) / (float)(_max-_min)) * 100.0f; // Map to 0-100%
  return moisture_percentage;
}

float read_moisture_analogic() {
  return (float)raw_moisture_value;
}