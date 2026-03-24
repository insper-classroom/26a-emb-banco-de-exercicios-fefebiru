#include <stdio.h>

// Bibliotecas do SDK do Raspberry Pi Pico
#include "hardware/adc.h"    // conversor analógico-digital (não usado diretamente aqui)
#include "hardware/gpio.h"   // controle de pinos GPIO
#include "hardware/timer.h"  // timers e alarmes de hardware
#include "pico/stdlib.h"     // funções padrão do Pico (stdio, sleep, etc.)

// Pinos dos botões
const int BTN_PIN_G = 28;  // botão verde no GPIO 28
const int BTN_PIN_Y = 26;  // botão amarelo no GPIO 26

// Pinos dos LEDs
const int LED_PIN_G = 5;   // LED verde no GPIO 5
const int LED_PIN_Y = 9;   // LED amarelo no GPIO 9

// Flags de botão — setadas pela interrupção quando o botão é pressionado
// volatile: impede o compilador de otimizar, pois o valor muda em interrupções
volatile int btn_g = 0;  // 1 = botão verde foi pressionado
volatile int btn_y = 0;  // 1 = botão amarelo foi pressionado

// Flags de alarme — setadas quando o alarme one-shot expira
volatile int alarm_g = 0;  // 1 = alarme verde expirou (tempo do LED acabou)
volatile int timer_g = 0;  // 1 = timer repetitivo verde disparou (hora de piscar)

// Flags de alarme e timer para o amarelo
volatile int alarm_y = 0;  // 1 = alarme amarelo expirou
volatile int timer_y = 0;  // 1 = timer repetitivo amarelo disparou


// Callback de interrupção dos botões
// Chamada automaticamente pelo hardware ao detectar borda no pino
void btn_callback(uint gpio, uint32_t events) {
    if (events == 0x4) {  // 0x4 = GPIO_IRQ_EDGE_FALL = borda de descida (botão pressionado)
        if (gpio == BTN_PIN_G) {
            btn_g = 1;  // sinaliza que o botão verde foi pressionado
        } else if (gpio == BTN_PIN_Y) {
            btn_y = 1;  // sinaliza que o botão amarelo foi pressionado
        }
    }
}

// Callback do alarme one-shot verde
// Chamada uma única vez após 1000ms do pressionar do botão verde
int64_t alarm_callback_g(alarm_id_t id, void *user_data) {
    alarm_g = 1;  // sinaliza que o tempo do LED verde expirou
    return 0;     // retornar 0 = não reagendar o alarme
}

// Callback do timer repetitivo verde
// Chamada a cada intervalo configurado (100ms) enquanto o timer estiver ativo
bool timer_callback_g(repeating_timer_t *rt){
    timer_g = 1;   // sinaliza que é hora de alternar o estado do LED verde
    return true;   // retornar true = continuar repetindo
}

// Callback do alarme one-shot amarelo
// Chamada uma única vez após 2000ms do pressionar do botão amarelo
int64_t alarm_callback_y(alarm_id_t id, void *user_data) {
    alarm_y = 1;  // sinaliza que o tempo do LED amarelo expirou
    return 0;     // retornar 0 = não reagendar o alarme
}

// Callback do timer repetitivo amarelo
// Chamada a cada intervalo configurado (250ms) enquanto o timer estiver ativo
bool timer_callback_y(repeating_timer_t *rt){
    timer_y = 1;   // sinaliza que é hora de alternar o estado do LED amarelo
    return true;   // retornar true = continuar repetindo
}

// Inicializa todos os periféricos de hardware
void setup(){
    // Configura botão verde: entrada com pull-up interno
    // Pull-up = pino em HIGH no repouso, vai para LOW quando pressionado
    gpio_init(BTN_PIN_G);
    gpio_set_dir(BTN_PIN_G, GPIO_IN);
    gpio_pull_up(BTN_PIN_G);

    // Configura botão amarelo: mesma lógica
    gpio_init(BTN_PIN_Y);
    gpio_set_dir(BTN_PIN_Y, GPIO_IN);
    gpio_pull_up(BTN_PIN_Y);

    // Configura LED verde como saída
    gpio_init(LED_PIN_G);
    gpio_set_dir(LED_PIN_G, GPIO_OUT);

    // Configura LED amarelo como saída
    gpio_init(LED_PIN_Y);
    gpio_set_dir(LED_PIN_Y, GPIO_OUT);

    // Habilita interrupção de borda de descida nos dois botões
    // Ambos compartilham a mesma função callback (btn_callback)
    gpio_set_irq_enabled_with_callback(BTN_PIN_G, GPIO_IRQ_EDGE_FALL, true, &btn_callback);
    gpio_set_irq_enabled_with_callback(BTN_PIN_Y, GPIO_IRQ_EDGE_FALL, true, &btn_callback);
}

int main() {
    stdio_init_all();  // inicializa USB/UART para printf
    setup();           // configura GPIOs e interrupções

    // Identificadores dos alarmes one-shot (usados para cancelar se necessário)
    alarm_id_t alarm_id_g = 0;
    repeating_timer_t timer_id_g;  // estrutura do timer repetitivo verde

    alarm_id_t alarm_id_y = 0;
    repeating_timer_t timer_id_y;  // estrutura do timer repetitivo amarelo

    // Flags de controle de estado (usadas no loop, não em interrupções)
    int alarm_g_ativado = 0;  // 1 = LED verde está em modo pisca-pisca ativo

    int alarm_y_ativado = 0;  // 1 = LED amarelo está em modo pisca-pisca ativo

    // Estado atual dos LEDs (0 = apagado, 1 = aceso)
    int led_state_g = 0;
    int led_state_y = 0;

    while (1) {

        // Se o timer verde disparou E o pisca-pisca verde está ativo: alterna o LED verde
        if (timer_g && alarm_g_ativado){
            timer_g = 0;                          // consome a flag do timer
            led_state_g = !led_state_g;           // inverte o estado do LED
            gpio_put(LED_PIN_G, led_state_g);     // aplica o novo estado no pino
        }

        // Se o timer amarelo disparou E o pisca-pisca amarelo está ativo: alterna o LED amarelo
        if (timer_y && alarm_y_ativado) {
            timer_y = 0;                          // consome a flag do timer
            led_state_y = !led_state_y;           // inverte o estado do LED
            gpio_put(LED_PIN_Y, led_state_y);     // aplica o novo estado no pino
        }

        // Se o botão verde foi pressionado E o pisca-pisca ainda não está ativo
        if (btn_g && !alarm_g_ativado){
            btn_g = 0;  // consome a flag do botão
            // Agenda alarme one-shot para daqui 1000ms (vai setar alarm_g quando expirar)
            alarm_id_g = add_alarm_in_ms(1000, alarm_callback_g, NULL, true);
            alarm_g_ativado = 1;  // marca que o pisca-pisca verde está ativo
            // Inicia timer repetitivo a cada 100ms para piscar o LED verde
            add_repeating_timer_ms(100, timer_callback_g, NULL, &timer_id_g);
        }

        // Se o botão amarelo foi pressionado E o pisca-pisca ainda não está ativo
        if (btn_y && !alarm_y_ativado){
            btn_y = 0;  // consome a flag do botão
            // Agenda alarme one-shot para daqui 2000ms (vai setar alarm_y quando expirar)
            alarm_id_y = add_alarm_in_ms(2000, alarm_callback_y, NULL, true);
            alarm_y_ativado = 1;  // marca que o pisca-pisca amarelo está ativo
            // Inicia timer repetitivo a cada 250ms para piscar o LED amarelo
            add_repeating_timer_ms(250, timer_callback_y, NULL, &timer_id_y);
        }

        // Se qualquer alarme expirou: desliga tudo (ambos os LEDs param juntos)
        if (alarm_y || alarm_g){
            // Zera todas as flags de estado
            alarm_y = 0;
            alarm_g = 0;
            alarm_g_ativado = 0;
            alarm_y_ativado = 0;
            timer_g = 0;
            timer_y = 0;
            // Cancela os alarmes e timers que ainda possam estar pendentes
            cancel_alarm(alarm_id_g);
            cancel_alarm(alarm_id_y);
            cancel_repeating_timer(&timer_id_g);
            cancel_repeating_timer(&timer_id_y);
            // Apaga os dois LEDs
            gpio_put(LED_PIN_G, 0);
            gpio_put(LED_PIN_Y, 0);
        }
    }

    return 0;
}
