
#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/gpio.h"
#include "hardware/timer.h"
#include "hardware/adc.h"

const int LED_AMARELO_PIN = 5;
const int LED_AZUL_PIN = 9;

const int BTN_PIN = 28;

volatile int flag_botao = 0;

volatile int timer_azul = 0;
volatile int timer_amarelo = 0;

volatile int alarme = 0;

void btn_callback(uint gpio, uint32_t events) {
    if (events == 0x4) {  // 0x4 = GPIO_IRQ_EDGE_FALL = borda de descida (botão pressionado)
       flag_botao = 1;
    }
}

int64_t alarm_callback(alarm_id_t id, void *user_data) {
    alarme = 1;
    return 0; 
}

bool timer_callback_azul(repeating_timer_t *rt){
    timer_azul = 1;   
    return true;
}

bool timer_callback_amarelo(repeating_timer_t *rt){
    timer_amarelo = 1;   
    return true;
}


void setup(){
    gpio_init(BTN_PIN);
    gpio_set_dir(BTN_PIN, GPIO_IN);
    gpio_pull_up(BTN_PIN);

    // Configura LED verde como saída
    gpio_init(LED_AZUL_PIN);
    gpio_set_dir(LED_AZUL_PIN, GPIO_OUT);

    // Configura LED amarelo como saída
    gpio_init(LED_AMARELO_PIN);
    gpio_set_dir(LED_AMARELO_PIN, GPIO_OUT);

    // Habilita interrupção de borda de descida nos dois botões
    // Ambos compartilham a mesma função callback (btn_callback)
    gpio_set_irq_enabled_with_callback(BTN_PIN, GPIO_IRQ_EDGE_FALL, true, &btn_callback);
}

int main() {
    stdio_init_all();
    setup();

    alarm_id_t alarm_id = 0;

    repeating_timer_t timer_id_azul;  
    repeating_timer_t timer_id_amarelo;

    int led_state_amarelo = 0;
    int led_state_azul = 0;

    while (true) {
        
        if (timer_amarelo && !alarme){
            timer_amarelo = 0;
            led_state_amarelo = !led_state_amarelo;
            gpio_put(LED_AMARELO_PIN, led_state_amarelo);
        }

        if (timer_azul && !alarme){
            timer_azul = 0;
            led_state_azul = !led_state_azul;
            gpio_put(LED_AZUL_PIN, led_state_azul);
        }

        if (flag_botao){
            flag_botao = 0;

            led_state_amarelo = 1;
            led_state_azul = 1;
            gpio_put(LED_AMARELO_PIN, 1);
            gpio_put(LED_AZUL_PIN, 1);            

            alarm_id = add_alarm_in_ms(5000, alarm_callback, NULL, true);
            add_repeating_timer_ms(150, timer_callback_azul, NULL, &timer_id_azul);
            add_repeating_timer_ms(500, timer_callback_amarelo, NULL, &timer_id_amarelo);
        }

        if (alarme){
            led_state_amarelo = 0;
            led_state_azul = 0;
            alarme = 0;
            timer_amarelo = 0;
            timer_azul = 0;
            cancel_alarm(alarm_id);
            cancel_repeating_timer(&timer_id_azul);
            cancel_repeating_timer(&timer_id_amarelo);
            gpio_put(LED_AZUL_PIN, 0);
            gpio_put(LED_AMARELO_PIN, 0);
        }

    }
}
