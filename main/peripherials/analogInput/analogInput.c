#include "esp_adc/adc_oneshot.h"
#include "esp_adc/adc_cali.h"
#include "esp_adc/adc_cali_scheme.h"
#include "analogInput.h"
#include "esp_log.h"

static adc_oneshot_unit_handle_t adc_handle = NULL;
static adc_cali_handle_t adc_cali_handle = NULL;
static bool adc_cali_enabled = false;

void analog_input_init(adc_unit_t unit, adc_channel_t channel, adc_atten_t atten) {
  adc_oneshot_unit_init_cfg_t init_cfg = {
    .unit_id = unit,
    .ulp_mode = ADC_ULP_MODE_DISABLE
  };
  if (adc_handle == NULL) {
    ESP_ERROR_CHECK(adc_oneshot_new_unit(&init_cfg, &adc_handle));
  }

  adc_oneshot_chan_cfg_t chan_cfg = {
    .bitwidth = ADC_BITWIDTH_12,
    .atten = atten
  };
  ESP_ERROR_CHECK(adc_oneshot_config_channel(adc_handle, channel, &chan_cfg));

  // Calibrazione (opzionale ma consigliata)
  adc_cali_line_fitting_config_t cali_cfg = {
    .unit_id = unit,
    .atten = atten,
    .bitwidth = ADC_BITWIDTH_12
  };
  if (adc_cali_create_scheme_line_fitting(&cali_cfg, &adc_cali_handle) == ESP_OK) {
    adc_cali_enabled = true;
  }
}

// Leggi analog input come tensione (Volt)
float read_analog_input(adc_unit_t unit, adc_channel_t channel) {
  int raw = 0;
  ESP_ERROR_CHECK(adc_oneshot_read(adc_handle, channel, &raw));
  if (adc_cali_enabled) {
    int voltage = 0;
    if (adc_cali_raw_to_voltage(adc_cali_handle, raw, &voltage) == ESP_OK) {
      //ESP_LOGI("ANALOG_INPUT", "Raw: %d, Voltage: %d mV, channel: %d", raw, voltage, channel);
      return voltage / 1000.0f;
    }
  }
  // Fallback: stima lineare
  return (raw / 4095.0f) * 3.3f;
}
