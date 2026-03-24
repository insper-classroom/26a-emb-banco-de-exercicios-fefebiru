#include "pico/stdlib.h"
#include <stdio.h>
#include "hardware/gpio.h"

#define BTN_PIN 22
#define SW_PIN 28
#define LED_1 2
#define LED_2 3
#define LED_3 4
#define LED_4 5
#define LED_5 6

void bar_init() {
    gpio_init(LED_1);
    gpio_set_dir(LED_1, GPIO_OUT);

    gpio_init(LED_2);
    gpio_set_dir(LED_2, GPIO_OUT);

    gpio_init(LED_3);
    gpio_set_dir(LED_3, GPIO_OUT);

    gpio_init(LED_4);
    gpio_set_dir(LED_4, GPIO_OUT);

    gpio_init(LED_5);
    gpio_set_dir(LED_5, GPIO_OUT);
}

void bar_display(int val) {
    gpio_put(LED_1, val >=1);
    gpio_put(LED_2, val >=2);
    gpio_put(LED_3, val >=3);
    gpio_put(LED_4, val >=4);
    gpio_put(LED_5, val >=5);
}

volatile bool flag_btn = false;
volatile bool sw_state = false;

void gpio_callback(uint gpio, uint32_t events) {
    if (gpio == BTN_PIN) {
        flag_btn = true;
    }
    if (gpio == SW_PIN) {
        sw_state = (events & GPIO_IRQ_EDGE_FALL);
    }
}

int main() {
    stdio_init_all();

    gpio_init(SW_PIN);
    gpio_set_dir(SW_PIN, GPIO_IN);
    gpio_pull_up(SW_PIN);

    bar_init();
    
    gpio_set_irq_enabled_with_callback(BTN_PIN, GPIO_IRQ_EDGE_FALL, true, &gpio_callback);
    gpio_set_irq_enabled(SW_PIN, GPIO_IRQ_EDGE_FALL | GPIO_IRQ_EDGE_RISE, true);

    int contador = 0;

    while (true) {
    
        if (flag_btn) {
            flag_btn = false;
            if (sw_state == false) {
                contador++;
            } else {
                contador--;
            }
            if (contador > 5) {
                contador = 5;
            }
            if (contador <0) {
                contador = 0;
            }

            bar_display(contador);
        }

    }
}