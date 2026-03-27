#include "pico/stdlib.h"
#include <stdio.h>

#include "hardware/gpio.h"

#define LED_R 2
#define LED_G 9
#define LED_P 26
#define BTN_G 28
#define BTN_B 27
#define BTN_Y 21
#define BTN_W 17

volatile bool flag_btn_g = false;
volatile bool flag_btn_b = false;
volatile bool flag_btn_y = false;
volatile bool flag_btn_w = false;

int senha[4];
int tentativa[4];
int contador = 0;

typedef enum {CONFIGURANDO, VERIFICANDO} state_t;
state_t state = CONFIGURANDO;

void btn_callback(uint gpio, uint32_t events) {
    if (events & GPIO_IRQ_EDGE_FALL) {
        if (gpio == BTN_G) flag_btn_g = true;
        else if (gpio == BTN_B) flag_btn_b = true;
        else if (gpio == BTN_Y) flag_btn_y = true;
        else if (gpio == BTN_W) flag_btn_w = true;
    }
}

int main() {
    stdio_init_all();

    gpio_init(BTN_G);
    gpio_set_dir(BTN_G, GPIO_IN);
    gpio_pull_up(BTN_G);

    gpio_init(BTN_B);
    gpio_set_dir(BTN_B, GPIO_IN);
    gpio_pull_up(BTN_B);

    gpio_init(BTN_Y);
    gpio_set_dir(BTN_Y, GPIO_IN);
    gpio_pull_up(BTN_Y);

    gpio_init(BTN_W);
    gpio_set_dir(BTN_W, GPIO_IN);
    gpio_pull_up(BTN_W);

    gpio_init(LED_G);
    gpio_set_dir(LED_G, GPIO_OUT);

    gpio_init(LED_R);
    gpio_set_dir(LED_R, GPIO_OUT);

    gpio_init(LED_P);
    gpio_set_dir(LED_P, GPIO_OUT);

    gpio_set_irq_enabled_with_callback(BTN_G, GPIO_IRQ_EDGE_FALL, true, &btn_callback);
    gpio_set_irq_enabled(BTN_Y, GPIO_IRQ_EDGE_FALL, true);
    gpio_set_irq_enabled(BTN_B, GPIO_IRQ_EDGE_FALL, true);
    gpio_set_irq_enabled(BTN_W, GPIO_IRQ_EDGE_FALL, true);

    while (1) {
        int btn_pressed = 0;
        if (flag_btn_g) {
            flag_btn_g = false;
            btn_pressed = BTN_G;
        } else if (flag_btn_b) {
            flag_btn_b = false;
            btn_pressed = BTN_B;
        } else if (flag_btn_y) {
            flag_btn_y = false;
            btn_pressed = BTN_Y;
        } else if (flag_btn_w) {
            flag_btn_w = false;
            btn_pressed = BTN_W;
        }

        if (btn_pressed == 0) continue;
    

        gpio_put(LED_P, 1);
        sleep_ms(100);
        gpio_put(LED_P, 0);
        
        if (state == CONFIGURANDO) {
            senha[contador] = btn_pressed;
            contador++;
            if (contador == 4) {
                gpio_put(LED_G, 1);
                sleep_ms(300);
                gpio_put(LED_G, 0);
                contador = 0;
                state = VERIFICANDO;
            }
        } else {
            tentativa[contador] = btn_pressed;
            contador++;
            if (contador == 4) {
                bool correta = true;
                for (int i = 0; i < 4; i++) {
                    if (tentativa[i] != senha[i]) {
                        correta = false;
                        break;
                    }
                }
                if (correta) {
                    gpio_put(LED_G, 1);
                    sleep_ms(300);
                    gpio_put(LED_G, 0);
                } else {
                    gpio_put(LED_R, 1);
                    sleep_ms(300);
                    gpio_put(LED_R,0);
                }
                contador = 0;
            }   
        }
    }
}