#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/gpio.h"
#include "hardware/timer.h"

const int LED_PIN_AMARELO = 10;
const int LED_PIN_AZUL = 14;

const int BTN_PIN_AMARELO = 26;
const int BTN_PIN_AZUL = 19;

volatile int flag_btn_azul = 0;
volatile int flag_btn_amarelo = 0;

volatile int alarme_azul = 0;
volatile int alarme_amarelo = 0;

volatile int timer_azul = 0;
volatile int timer_amarelo = 0;

repeating_timer_t timer_id_azul;
repeating_timer_t timer_id_amarelo;

alarm_id_t alarm_id_azul = 0;
alarm_id_t alarm_id_amarelo = 0;

void btn_callback(uint gpio, uint32_t events) {
    if (events == 0x4) {
        if (gpio == BTN_PIN_AZUL) {
            flag_btn_azul = 1;
        } else if (gpio == BTN_PIN_AMARELO) {
            flag_btn_amarelo = 1;
        }
    }
}

int64_t alarm_callback_azul(alarm_id_t id, void *user_data) {
    alarme_azul = 1;
    return 0;
}

int64_t alarm_callback_amarelo(alarm_id_t id, void *user_data) {
    alarme_amarelo = 1;
    return 0;
}

bool timer_callback_azul(repeating_timer_t *rt) {
    timer_azul = 1;
    return true;
}

bool timer_callback_amarelo(repeating_timer_t *rt) {
    timer_amarelo = 1;
    return true;
}

void setup() {
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

    gpio_set_irq_enabled_with_callback(BTN_PIN_AMARELO, GPIO_IRQ_EDGE_FALL, true, &btn_callback);
    gpio_set_irq_enabled_with_callback(BTN_PIN_AZUL, GPIO_IRQ_EDGE_FALL, true, &btn_callback);
}

int main() {
    stdio_init_all();
    setup();

    int led_state_amarelo = 0;
    int led_state_azul = 0;
    int segunda_fase = 0;

    while (1) {

        // --- toggle dos LEDs ---
        if (timer_amarelo) {
            timer_amarelo = 0;
            led_state_amarelo = !led_state_amarelo;
            gpio_put(LED_PIN_AMARELO, led_state_amarelo);
        }

        if (timer_azul) {
            timer_azul = 0;
            led_state_azul = !led_state_azul;
            gpio_put(LED_PIN_AZUL, led_state_azul);
        }

        // --- botão amarelo: amarelo (1s) → azul (2s) ---
        if (flag_btn_amarelo) {
            flag_btn_amarelo = 0;
            segunda_fase = 0;
            cancel_repeating_timer(&timer_id_amarelo);
            cancel_repeating_timer(&timer_id_azul);
            cancel_alarm(alarm_id_amarelo);
            cancel_alarm(alarm_id_azul);
            gpio_put(LED_PIN_AMARELO, 0);
            gpio_put(LED_PIN_AZUL, 0);
            led_state_amarelo = 0;
            led_state_azul = 0;
            alarm_id_amarelo = add_alarm_in_ms(1000, alarm_callback_amarelo, NULL, true);
            add_repeating_timer_ms(200, timer_callback_amarelo, NULL, &timer_id_amarelo);
        }

        // --- botão azul: azul (2s) → amarelo (1s) ---
        if (flag_btn_azul) {
            flag_btn_azul = 0;
            segunda_fase = 0;
            cancel_repeating_timer(&timer_id_amarelo);
            cancel_repeating_timer(&timer_id_azul);
            cancel_alarm(alarm_id_amarelo);
            cancel_alarm(alarm_id_azul);
            gpio_put(LED_PIN_AMARELO, 0);
            gpio_put(LED_PIN_AZUL, 0);
            led_state_amarelo = 0;
            led_state_azul = 0;
            alarm_id_azul = add_alarm_in_ms(2000, alarm_callback_azul, NULL, true);
            add_repeating_timer_ms(500, timer_callback_azul, NULL, &timer_id_azul);
        }

        // --- alarme amarelo disparou ---
        if (alarme_amarelo) {
            if (!segunda_fase) {
                // primeira fase: para amarelo, inicia azul
                cancel_repeating_timer(&timer_id_amarelo);
                cancel_repeating_timer(&timer_id_azul);
                cancel_alarm(alarm_id_azul);
                gpio_put(LED_PIN_AMARELO, 0);
                led_state_amarelo = 0;
                alarme_amarelo = 0;
                alarm_id_azul = add_alarm_in_ms(2000, alarm_callback_azul, NULL, true);
                add_repeating_timer_ms(500, timer_callback_azul, NULL, &timer_id_azul);
                segunda_fase = 1;
            } else {
                // segunda fase: para amarelo, FIM
                cancel_repeating_timer(&timer_id_amarelo);
                cancel_alarm(alarm_id_amarelo);
                gpio_put(LED_PIN_AMARELO, 0);
                led_state_amarelo = 0;
                alarme_amarelo = 0;
                segunda_fase = 0;
            }
        }

        // --- alarme azul disparou ---
        if (alarme_azul) {
            if (!segunda_fase) {
                // primeira fase: para azul, inicia amarelo
                cancel_repeating_timer(&timer_id_azul);
                cancel_repeating_timer(&timer_id_amarelo);
                cancel_alarm(alarm_id_amarelo);
                gpio_put(LED_PIN_AZUL, 0);
                led_state_azul = 0;
                alarme_azul = 0;
                alarm_id_amarelo = add_alarm_in_ms(1000, alarm_callback_amarelo, NULL, true);
                add_repeating_timer_ms(200, timer_callback_amarelo, NULL, &timer_id_amarelo);
                segunda_fase = 1;
            } else {
                // segunda fase: para azul, FIM
                cancel_repeating_timer(&timer_id_azul);
                cancel_alarm(alarm_id_azul);
                gpio_put(LED_PIN_AZUL, 0);
                led_state_azul = 0;
                alarme_azul = 0;
                segunda_fase = 0;
            }
        }
    }

    return 0;
}