#ifndef MOISTURE_H
#define MOISTURE_H

#include "HMI.h"
#include <driver/gpio.h>
#include "sclib/hmi_tools/hmi_tools.h"

#define MOISTURE_SENSOR_PIN GPIO_NUM_39
#define MOISTURE_SENSOR_ADC_CHANNEL ADC1_CHANNEL_3

void moisture_init();
float read_moisture_level(int min, int max);
float read_moisture_analogic();

#endif