/**
 * Copyright (c) 2020 Raspberry Pi (Trading) Ltd.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <stdio.h>
#include <string.h> 
#include "pico/stdlib.h"
#include "hardware/gpio.h"
#include "hardware/timer.h"
#include "hardware/irq.h"

const int BTN_PIN = 28;
const int LED_PIN = 5;

volatile int rise = 0;
volatile int fall = 0;

volatile int timer = 0;


void btn_callback(uint gpio, uint32_t events) {
    if (events == 0x4) {  // 0x4 = GPIO_IRQ_EDGE_FALL = borda de descida (botão pressionado)
       fall = 1;
    } else if (events == 0x8) { // 0x8 = GPIO_IRQ_EDGE_RISE = borda de subida (botão levantado)
        rise = 1;
    }
}

bool timer_callback(repeating_timer_t *rt){
    timer = 1;   
    return true;
}

void setup(){
    gpio_init(BTN_PIN);
    gpio_set_dir(BTN_PIN, GPIO_IN);
    gpio_pull_up(BTN_PIN);

    gpio_init(LED_PIN);
    gpio_set_dir(LED_PIN, GPIO_OUT);

    gpio_set_irq_enabled_with_callback(BTN_PIN, GPIO_IRQ_EDGE_FALL | GPIO_IRQ_EDGE_RISE, true, &btn_callback);
}

int main() {
    stdio_init_all();
    setup();


    absolute_time_t tempo_i;
    absolute_time_t tempo_f;

    repeating_timer_t timer_id;

    int led_state = 0;

    while (true) {

        if (timer){
            timer = 0;
            led_state = !led_state;
            gpio_put(LED_PIN, led_state);
        }

        if (fall){
            fall = 0;
            tempo_i = get_absolute_time();
            led_state = 0;
            gpio_put(LED_PIN,0);
            cancel_repeating_timer(&timer_id);
        }

        if (rise){
            rise = 0;
            tempo_f = get_absolute_time();
            int64_t duration = absolute_time_diff_us(tempo_i,tempo_f);
            led_state = 1;
            gpio_put(LED_PIN, 1);
            add_repeating_timer_us(duration, timer_callback, NULL, &timer_id);
        }
    }
    return 0;
}