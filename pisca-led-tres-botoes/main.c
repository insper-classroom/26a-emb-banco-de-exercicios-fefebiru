#include "pico/stdlib.h"
#include <stdio.h>
#include "hardware/gpio.h"

const int LED_PIN_AZUL = 10;
const int LED_PIN_VERDE = 14;
const int LED_PIN_AMARELO = 6;

const int BTN_PIN_AZUL = 22;
const int BTN_PIN_VERDE = 18;
const int BTN_PIN_AMARELO = 28;

volatile int flag_btn_amarelo = 0;
volatile int flag_btn_azul = 0;

volatile int fall_btn_verde = 0;
volatile int rise_btn_verde = 0;

void btn_callback(uint gpio, uint32_t events) {
  if (events == 0x4) {  // 0x4 = GPIO_IRQ_EDGE_FALL = borda de descida (botão pressionado)
    if (gpio == BTN_PIN_VERDE) {
      fall_btn_verde = 1;
    } else if (gpio == BTN_PIN_AMARELO) {
      flag_btn_amarelo = 1;
    } 
  } else if (events == 0x8){
    if (gpio == BTN_PIN_VERDE){
      rise_btn_verde = 1;
    } else if (gpio == BTN_PIN_AZUL){
      flag_btn_azul = 1;
    }
  }
}

void setup(){
    gpio_init(BTN_PIN_AMARELO);
    gpio_set_dir(BTN_PIN_AMARELO, GPIO_IN);
    gpio_pull_up(BTN_PIN_AMARELO);

    gpio_init(LED_PIN_AMARELO);
    gpio_set_dir(LED_PIN_AMARELO, GPIO_OUT);

    gpio_init(BTN_PIN_AZUL);
    gpio_set_dir(BTN_PIN_AZUL, GPIO_IN);
    gpio_pull_up(BTN_PIN_AZUL);

    gpio_init(LED_PIN_AZUL);
    gpio_set_dir(LED_PIN_AZUL, GPIO_OUT);
    
    gpio_init(BTN_PIN_VERDE);
    gpio_set_dir(BTN_PIN_VERDE, GPIO_IN);
    gpio_pull_up(BTN_PIN_VERDE);

    gpio_init(LED_PIN_VERDE);
    gpio_set_dir(LED_PIN_VERDE, GPIO_OUT);
    
    
    gpio_set_irq_enabled_with_callback(BTN_PIN_AMARELO, GPIO_IRQ_EDGE_FALL, true, &btn_callback);
    gpio_set_irq_enabled_with_callback(BTN_PIN_AZUL, GPIO_IRQ_EDGE_RISE, true, &btn_callback);
    gpio_set_irq_enabled_with_callback(BTN_PIN_VERDE, GPIO_IRQ_EDGE_FALL | GPIO_IRQ_EDGE_RISE, true, &btn_callback);



}

int main() {
  stdio_init_all();
  setup();
  

  int led_state_amarelo = 0;
  int led_state_azul = 0;
  int led_state_verde = 0;

  int piscando_amarelo = 0;
  int piscando_azul = 0;
  int piscando_verde = 0;

  while (true) {

    if (flag_btn_amarelo){
      flag_btn_amarelo = 0;
      piscando_amarelo = !piscando_amarelo;
      
      if (!piscando_amarelo){
        gpio_put(LED_PIN_AMARELO,0);
      }
    }

    if (flag_btn_azul){
      flag_btn_azul = 0;
      piscando_azul = !piscando_azul;
      
      if (!piscando_azul){
        gpio_put(LED_PIN_AZUL,0);
      }
    }

    if (fall_btn_verde){
        fall_btn_verde = 0;
        piscando_verde = 1;
    }

    if (rise_btn_verde){
      rise_btn_verde = 0;
      piscando_verde = 0;
      gpio_put(LED_PIN_VERDE,0);
    }


    if (piscando_amarelo){
      led_state_amarelo = !led_state_amarelo;
      gpio_put(LED_PIN_AMARELO,led_state_amarelo);
    }

    if (piscando_azul){
      led_state_azul = !led_state_azul;
      gpio_put(LED_PIN_AZUL,led_state_azul);
    }

    if (piscando_verde){
      led_state_verde = !led_state_verde;
      gpio_put(LED_PIN_VERDE,led_state_verde);
    }

    sleep_ms(200);
  }
}