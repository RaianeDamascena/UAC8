#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/adc.h"
#include "hardware/pwm.h"
#include "hardware/i2c.h"
#include "lib/ssd1306.h"
#include "lib/font.h"

// Definições de pinos e configuração do I2C
#define I2C_PORT i2c1
#define I2C_SDA 14
#define I2C_SCL 15
#define ENDERECO 0x3C
#define JOYSTICK_X_PIN 27
#define JOYSTICK_Y_PIN 26
#define JOYSTICK_PB 22
#define BOTAO_A 5
#define LED_PIN_RED 13
#define LED_PIN_GREEN 11
#define LED_PIN_BLUE 12

// Variáveis globais
static volatile uint32_t last_time = 0;
uint pwm_slice_red = 0;
uint pwm_slice_blue = 0;
ssd1306_t ssd;
bool cor = true;
bool leds_pwm_ativados = true;
bool leds_pwm_mudaram_de_estado = true;
bool joystick_pressionado = false;

// Função de interrupção para o gerenciamento de botões
void gpio_irq_handler(uint gpio, uint32_t events) {
    uint32_t current_time = to_us_since_boot(get_absolute_time());
    
    // Verifica o tempo para debounce (200ms)
    if (current_time - last_time > 200000) {
        last_time = current_time;
        
        if (gpio == JOYSTICK_PB) {
            // Alterna o LED verde
            gpio_put(LED_PIN_GREEN, !gpio_get(LED_PIN_GREEN));
            printf("LED verde alternado!\n");

            // Alterna o estado do botão do joystick
            joystick_pressionado = !joystick_pressionado;
        }
        
        if (gpio == BOTAO_A) {
            // Alterna o estado dos LEDs PWM
            if (leds_pwm_ativados) {
                pwm_set_enabled(pwm_slice_red, 0);  // Desativa PWM do LED vermelho
                pwm_set_enabled(pwm_slice_blue, 0); // Desativa PWM do LED azul
                leds_pwm_ativados = false;
            } else {
                pwm_set_enabled(pwm_slice_red, 1);  // Ativa PWM do LED vermelho
                pwm_set_enabled(pwm_slice_blue, 1); // Ativa PWM do LED azul
                leds_pwm_ativados = true;
            }
            leds_pwm_mudaram_de_estado = true;
        }
    }
}

// Função para inicializar o PWM nos LEDs
uint pwm_init_gpio(uint gpio, uint wrap) {
    gpio_set_function(gpio, GPIO_FUNC_PWM);
    uint slice_num = pwm_gpio_to_slice_num(gpio);
    pwm_set_wrap(slice_num, wrap);
    pwm_set_enabled(slice_num, true);
    return slice_num;
}

int main() {
    stdio_init_all();

    // Inicializa o botão A
    gpio_init(BOTAO_A);
    gpio_set_dir(BOTAO_A, GPIO_IN);
    gpio_pull_up(BOTAO_A);
    gpio_set_irq_enabled_with_callback(BOTAO_A, GPIO_IRQ_EDGE_FALL, true, &gpio_irq_handler);

    // Inicializa o botão do joystick (GPIO 22)
    gpio_init(JOYSTICK_PB);
    gpio_set_dir(JOYSTICK_PB, GPIO_IN);
    gpio_pull_up(JOYSTICK_PB);  // Ativa o pull-up para garantir que o botão não leia como pressionado o tempo todo
    gpio_set_irq_enabled_with_callback(JOYSTICK_PB, GPIO_IRQ_EDGE_FALL, true, &gpio_irq_handler);

    // Inicializa I2C
    i2c_init(I2C_PORT, 400 * 1000);
    gpio_set_function(I2C_SDA, GPIO_FUNC_I2C);
    gpio_set_function(I2C_SCL, GPIO_FUNC_I2C);
    gpio_pull_up(I2C_SDA);
    gpio_pull_up(I2C_SCL);

    // Inicializa o display SSD1306
    ssd1306_init(&ssd, WIDTH, HEIGHT, false, ENDERECO, I2C_PORT);
    ssd1306_config(&ssd);
    ssd1306_send_data(&ssd);
    ssd1306_fill(&ssd, false);
    ssd1306_send_data(&ssd);

    // Inicializa ADC para o joystick
    adc_init();
    adc_gpio_init(JOYSTICK_X_PIN);
    adc_gpio_init(JOYSTICK_Y_PIN);  

    // Inicializa LEDs
    gpio_init(LED_PIN_RED);
    gpio_set_dir(LED_PIN_RED, GPIO_OUT);
    pwm_set_gpio_level(LED_PIN_RED, 0);

    gpio_init(LED_PIN_GREEN);
    gpio_set_dir(LED_PIN_GREEN, GPIO_OUT);  
    pwm_set_gpio_level(LED_PIN_GREEN, 0); 

    gpio_init(LED_PIN_BLUE);
    gpio_set_dir(LED_PIN_BLUE, GPIO_OUT);  
    pwm_set_gpio_level(LED_PIN_BLUE, 0); 

    uint pwm_wrap = 4096;
    pwm_slice_red = pwm_init_gpio(LED_PIN_RED, pwm_wrap);
    pwm_slice_blue = pwm_init_gpio(LED_PIN_BLUE, pwm_wrap);

    uint16_t adc_value_x = 0;
    uint16_t adc_value_y = 0;
    uint8_t screen_x = 60;
    uint8_t screen_y = 29;

    while (true) {
        // Lê os valores do joystick
        adc_select_input(0); // Eixo Y
        adc_value_y = adc_read();
        adc_select_input(1); // Eixo X
        adc_value_x = adc_read();

        // Atualiza a posição do quadrado no display
        double screen_x16 = (((((double) adc_value_x)) / 4095) * 106) + 7;
        double screen_y16 = (((4095 - ((double) adc_value_y)) / 4095) * 42) + 7;  
        screen_x = (uint8_t) screen_x16;
        screen_y = (uint8_t) screen_y16;

        // Limpa o display
        ssd1306_fill(&ssd, !cor);

        // Desenha a borda normal ou pontilhada dependendo do estado do joystick
        if (!joystick_pressionado) {
            ssd1306_rect(&ssd, 6, 6, 115, 51, cor, !cor); // Borda normal
        } else {
            ssd1306_border_character(&ssd, 0, 0, 128, 64, cor, '.'); // Borda pontilhada
        }

        // Ajusta brilho dos LEDs conforme os valores do joystick
        pwm_set_gpio_level(LED_PIN_RED, (screen_x > 54 && screen_x < 64) ? 0 : adc_value_x);
        pwm_set_gpio_level(LED_PIN_BLUE, (screen_y > 22 && screen_y < 32) ? 0 : 4095 - adc_value_y);

        // Desenha o quadrado móvel
        ssd1306_rect(&ssd, screen_y, screen_x, 8, 8, cor, 1);
        ssd1306_send_data(&ssd);

        sleep_ms(100);
    }
}
