#ifndef ANALOG_INPUT_H
#define ANALOG_INPUT_H

#include "HMI.h"
#include <driver/gpio.h>
#include "sclib/hmi_tools/hmi_tools.h"


#include "esp_adc/adc_oneshot.h"

// Inizializza ADC oneshot e calibrazione per una specifica unità/canale
void analog_input_init(adc_unit_t unit, adc_channel_t channel, adc_atten_t atten);

// Legge la tensione (Volt) dal canale/unità specificati (già calibrata se possibile)
float read_analog_input(adc_unit_t unit, adc_channel_t channel);

#endif