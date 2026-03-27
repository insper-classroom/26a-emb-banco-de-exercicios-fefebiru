#include <stdio.h>
#include "hardware/gpio.h"
#include "pico/stdlib.h"

const int BTN_PIN_G = 28;
const int BTN_PIN_Y = 26;
const int BTN_PIN_R = 20;

const int LED_PIN_G = 5;
const int LED_PIN_Y = 9;
const int LED_PIN_R = 13;


volatile int btn_flag_G;
volatile int btn_flag_Y;
volatile int btn_flag_R;

volatile bool fired = false;


void btn_callback(uint gpio, uint32_t events) {
    if (events == 0x4) {  // rise edge
        if (gpio == BTN_PIN_G) {
            btn_flag_G = 1;   
        }
        if (gpio == BTN_PIN_Y) {
            btn_flag_Y = 1;   
        }
        if (gpio == BTN_PIN_R) {
            btn_flag_R = 1;   
        }
    }
}

int64_t alarm_callback(alarm_id_t id, void *user_data) {
    fired = true;
    return 0;
}

int main() {
    alarm_id_t alarm;
    int teste = 0;
    int atual = 0;
    int estado = 0;
    int level = 0;
    int seq[12];
    for (int i = 0; i < 12; i+=3){
        seq[i] = LED_PIN_Y;
        seq[i+1] = LED_PIN_G;
        seq[i+2] = LED_PIN_R;
    }
    // Printa o vetor completo
    for (int i = 0; i < 12; i++) {
        printf("seq[%d] = %d\n", i, seq[i]);
    } 
    int resp[12];
    for (int i = 0; i < 12; i+=3){
        resp[i] = BTN_PIN_Y;
        resp[i+1] = BTN_PIN_G;
        resp[i+2] = BTN_PIN_R;
    }
    // Printa o vetor completo
    for (int i = 0; i < 12; i++) {
        printf("resp[%d] = %d\n", i, resp[i]);
    }

    stdio_init_all();

    gpio_init(BTN_PIN_R);
    gpio_set_dir(BTN_PIN_R, GPIO_IN);
    gpio_pull_up(BTN_PIN_R);

    gpio_init(LED_PIN_R);
    gpio_set_dir(LED_PIN_R, GPIO_OUT);

    gpio_init(BTN_PIN_Y);
    gpio_set_dir(BTN_PIN_Y, GPIO_IN);
    gpio_pull_up(BTN_PIN_Y);

    gpio_init(LED_PIN_Y);
    gpio_set_dir(LED_PIN_Y, GPIO_OUT);

    gpio_init(BTN_PIN_G);
    gpio_set_dir(BTN_PIN_G, GPIO_IN);
    gpio_pull_up(BTN_PIN_G);

    gpio_init(LED_PIN_G);
    gpio_set_dir(LED_PIN_G, GPIO_OUT);


    
    
    gpio_set_irq_enabled_with_callback(BTN_PIN_R, GPIO_IRQ_EDGE_FALL, true,
                                      &btn_callback);

    // callback led g (nao usar _with_callback)
    gpio_set_irq_enabled(BTN_PIN_Y, GPIO_IRQ_EDGE_FALL, true);
    gpio_set_irq_enabled(BTN_PIN_G, GPIO_IRQ_EDGE_FALL, true);

    while (true) {
        if (btn_flag_G) {
            if (estado == 0){
                estado = 1;
                // printf("aqui0\n");
            }
            btn_flag_G = 0;
        }
        if (estado == 1){
            for (int j = 0; j <= level; j++){
                // printf("aqui1\n");
                gpio_put(seq[j], 1);
                sleep_ms(500);
                gpio_put(seq[j], 0);
            }
            estado = 4;
            atual = 0;
        }
        if (estado == 4){
            // printf("aqui4\n");
            alarm = add_alarm_in_ms(3000, alarm_callback, NULL, false);
            estado = 2;
            teste = 0;
        }
        if (estado == 2){
            if (teste == 0){
                // printf("%d", atual);
                // printf("esperado: %d\n", resp[atual]);
                teste = 1;
            }
        
            if (resp[atual] == BTN_PIN_Y) {
                if (btn_flag_Y){
                    atual++;
                    cancel_alarm(alarm);
                    estado = 4;
                    // printf("acertou\n");
                    btn_flag_Y = 0;
                    gpio_put(LED_PIN_Y, 1);
                    sleep_ms(300);
                    gpio_put(LED_PIN_Y, 0);
                }
                if (btn_flag_R){
                    // printf("errou\n");
                    estado = 3;
                    btn_flag_R = 0;
                }
                if (btn_flag_G){
                    // printf("errou\n");
                    estado = 3;
                    btn_flag_G = 0;
                }
            }
            if (resp[atual] == BTN_PIN_R) {
                if (btn_flag_R){
                    atual++;
                    cancel_alarm(alarm);
                    estado = 4;
                    // printf("acertou\n");
                    btn_flag_R = 0;
                    gpio_put(LED_PIN_R, 1);
                    sleep_ms(300);
                    gpio_put(LED_PIN_R, 0);
                }
                if (btn_flag_Y){
                    // printf("errou\n");
                    estado = 3;
                    btn_flag_Y = 0;
                }
                if (btn_flag_G){
                    // printf("errou\n");
                    estado = 3;
                    btn_flag_G = 0;
                }
            }
            if (resp[atual] == BTN_PIN_G) {
                if (btn_flag_G){
                    atual++;
                    cancel_alarm(alarm);
                    estado = 4;
                    // printf("acertou\n");
                    btn_flag_G = 0;
                    gpio_put(LED_PIN_G, 1);
                    sleep_ms(300);
                    gpio_put(LED_PIN_G, 0);
                }
                if (btn_flag_R){
                    // printf("errou\n");
                    estado = 3;
                    btn_flag_R = 0;
                }
                if (btn_flag_Y){
                    // printf("errou\n");
                    estado = 3;
                    btn_flag_Y = 0;
                }
            }
            if (atual > level){
                // printf("passou\n");
                atual = 0;
                estado = 1;
                level++;
                teste = 0;
                sleep_ms(500);
            }
            
        }
        if (fired){
            estado = 3;
        }
        if (estado == 3){
            cancel_alarm(alarm);
            estado = 0;
            fired = 0;
            int pnts = level;
            printf("Points %d\n", pnts);
            level = 0;
            pnts = 0;
        }
       
    }
}