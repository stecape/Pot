#include "LED.h"
#include "../../HMI.h"

static const char *TAG = "LED";

static volatile uint8_t inLevel = false;
static uint8_t inBtn = false, inPrev = false;
static uint16_t inTouch;

void led_setup(){
  gpio_reset_pin(GPIO_NUM_13);
  gpio_set_direction(GPIO_NUM_38, GPIO_MODE_INPUT);
  gpio_set_direction(GPIO_NUM_13, GPIO_MODE_OUTPUT);
  touch_pad_init();
  touch_pad_config(TOUCH_PAD_NUM0, 0);
}

void led_loop(){
  touch_pad_read(TOUCH_PAD_NUM0, &inTouch);
  inBtn = (gpio_get_level(GPIO_NUM_38) & (inTouch>300));
  
  if (inBtn != inPrev){
    inPrev = inBtn;
    if (inPrev){
      inLevel = !inLevel;               
      ESP_LOGI(TAG, "Turning the LED %s!", inLevel ? "ON" : "OFF");
    }
  }

  //gpio_set_level(GPIO_NUM_13, inLevel); //Original behavior before HMI

  //Led is controlled by HMI
  if (PLC.Faults.LowMoisture.Q == true){
    gpio_set_level(GPIO_NUM_13, true);
  } else {
    gpio_set_level(GPIO_NUM_13, false);
  }
}

void led_interrupt(){
  inLevel = !inLevel;
  //ESP_LOGI(TAG, "inLevel toggled by timer task: %d", inLevel);
}