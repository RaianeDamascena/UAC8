#include "lib/ssd1306.h"
#include "lib/font.h"

// Função de inicialização do display SSD1306
void ssd1306_init(ssd1306_t *ssd, uint8_t width, uint8_t height, bool external_vcc, uint8_t address, i2c_inst_t *i2c) {
    ssd->width = width;
    ssd->height = height;
    ssd->pages = height / 8U;
    ssd->address = address;
    ssd->i2c_port = i2c;
    ssd->bufsize = ssd->pages * ssd->width + 1;
    ssd->ram_buffer = calloc(ssd->bufsize, sizeof(uint8_t));
    ssd->ram_buffer[0] = 0x40;
    ssd->port_buffer[0] = 0x80;
}

// Função de configuração do display SSD1306
void ssd1306_config(ssd1306_t *ssd) {
    ssd1306_command(ssd, SET_DISP | 0x00);                // Desabilita o display
    ssd1306_command(ssd, SET_MEM_ADDR);                    // Seleciona o modo de endereçamento de memória
    ssd1306_command(ssd, 0x01);                             // Define a posição inicial do endereço
    ssd1306_command(ssd, SET_DISP_START_LINE | 0x00);      // Define a linha inicial do display
    ssd1306_command(ssd, SET_SEG_REMAP | 0x01);            // Inverte os segmentos
    ssd1306_command(ssd, SET_MUX_RATIO);                   // Define a razão de multiplexação
    ssd1306_command(ssd, HEIGHT - 1);                       // Altura do display
    ssd1306_command(ssd, SET_COM_OUT_DIR | 0x08);          // Direção de saída do COM
    ssd1306_command(ssd, SET_DISP_OFFSET);                 // Define o deslocamento do display
    ssd1306_command(ssd, 0x00);                             // Define o deslocamento para 0
    ssd1306_command(ssd, SET_COM_PIN_CFG);                 // Configura os pinos COM
    ssd1306_command(ssd, 0x12);                             // Configuração específica de pinos
    ssd1306_command(ssd, SET_DISP_CLK_DIV);                // Define o divisor do clock
    ssd1306_command(ssd, 0x80);                             // Define o valor do divisor
    ssd1306_command(ssd, SET_PRECHARGE);                   // Configura o tempo de pré-carga
    ssd1306_command(ssd, 0xF1);                             // Configuração do pré-carga
    ssd1306_command(ssd, SET_VCOM_DESEL);                  // Configura a seleção de VCOM
    ssd1306_command(ssd, 0x30);                             // Seleção de VCOM
    ssd1306_command(ssd, SET_CONTRAST);                    // Define o contraste
    ssd1306_command(ssd, 0xFF);                             // Valor máximo de contraste
    ssd1306_command(ssd, SET_ENTIRE_ON);                   // Ativa todos os pixels
    ssd1306_command(ssd, SET_NORM_INV);                    // Ativa a inversão normal
    ssd1306_command(ssd, SET_CHARGE_PUMP);                 // Ativa o circuito de carga
    ssd1306_command(ssd, 0x14);                             // Configuração do circuito de carga
    ssd1306_command(ssd, SET_DISP | 0x01);                 // Ativa o display
}

// Função para enviar um comando ao display
void ssd1306_command(ssd1306_t *ssd, uint8_t command) {
    ssd->port_buffer[1] = command;
    i2c_write_blocking(
        ssd->i2c_port,
        ssd->address,
        ssd->port_buffer,
        2,
        false
    );
}

// Função para enviar dados para o display
void ssd1306_send_data(ssd1306_t *ssd) {
    ssd1306_command(ssd, SET_COL_ADDR);
    ssd1306_command(ssd, 0);
    ssd1306_command(ssd, ssd->width - 1);
    ssd1306_command(ssd, SET_PAGE_ADDR);
    ssd1306_command(ssd, 0);
    ssd1306_command(ssd, ssd->pages - 1);
    i2c_write_blocking(
        ssd->i2c_port,
        ssd->address,
        ssd->ram_buffer,
        ssd->bufsize,
        false
    );
}

// Função para desenhar um pixel específico no display
void ssd1306_pixel(ssd1306_t *ssd, uint8_t x, uint8_t y, bool value) {
    uint16_t index = (y >> 3) + (x << 3) + 1;
    uint8_t pixel = (y & 0b111);
    if (value)
        ssd->ram_buffer[index] |= (1 << pixel);
    else
        ssd->ram_buffer[index] &= ~(1 << pixel);
}

// Função para preencher todo o display com um valor
void ssd1306_fill(ssd1306_t *ssd, bool value) {
    // Itera por todas as posições do display
    for (uint8_t y = 0; y < ssd->height; ++y) {
        for (uint8_t x = 0; x < ssd->width; ++x) {
            ssd1306_pixel(ssd, x, y, value);
        }
    }
}

// Função para desenhar um retângulo no display
void ssd1306_rect(ssd1306_t *ssd, uint8_t top, uint8_t left, uint8_t width, uint8_t height, bool value, bool fill) {
    // Desenha as bordas do retângulo
    for (uint8_t x = left; x < left + width; ++x) {
        ssd1306_pixel(ssd, x, top, value);
        ssd1306_pixel(ssd, x, top + height - 1, value);
    }
    for (uint8_t y = top; y < top + height; ++y) {
        ssd1306_pixel(ssd, left, y, value);
        ssd1306_pixel(ssd, left + width - 1, y, value);
    }

    // Preenche o retângulo se necessário
    if (fill) {
        for (uint8_t x = left + 1; x < left + width - 1; ++x) {
            for (uint8_t y = top + 1; y < top + height - 1; ++y) {
                ssd1306_pixel(ssd, x, y, value);
            }
        }
    }
}

// Função para desenhar uma linha entre dois pontos
void ssd1306_line(ssd1306_t *ssd, uint8_t x0, uint8_t y0, uint8_t x1, uint8_t y1, bool value) {
    int dx = abs(x1 - x0);
    int dy = abs(y1 - y0);

    int sx = (x0 < x1) ? 1 : -1;
    int sy = (y0 < y1) ? 1 : -1;

    int err = dx - dy;

    while (true) {
        ssd1306_pixel(ssd, x0, y0, value); // Desenha o pixel atual

        if (x0 == x1 && y0 == y1) break; // Termina quando alcança o ponto final

        int e2 = err * 2;

        if (e2 > -dy) {
            err -= dy;
            x0 += sx;
        }

        if (e2 < dx) {
            err += dx;
            y0 += sy;
        }
    }
}

// Função para desenhar uma linha horizontal
void ssd1306_hline(ssd1306_t *ssd, uint8_t x0, uint8_t x1, uint8_t y, bool value) {
    for (uint8_t x = x0; x <= x1; ++x)
        ssd1306_pixel(ssd, x, y, value);
}

// Função para desenhar uma linha vertical
void ssd1306_vline(ssd1306_t *ssd, uint8_t x, uint8_t y0, uint8_t y1, bool value) {
    for (uint8_t y = y0; y <= y1; ++y)
        ssd1306_pixel(ssd, x, y, value);
}

// Função para desenhar um caractere no display
void ssd1306_draw_char(ssd1306_t *ssd, char c, uint8_t x, uint8_t y) {
    uint16_t index = 0;
    if (c >= 'A' && c <= 'Z') {
        index = (c - 'A' + 11) * 8; // Para letras maiúsculas
    } else if (c >= 'a' && c <= 'z') {
        index = (c - 'a' + 37) * 8; // Para letras minúsculas
    } else if (c >= '0' && c <= '9') {
        index = (c - '0' + 1) * 8; // Para números
    } else if (c >= '[' && c <= '_') {
        index = (c - '[' + 63) * 8; // Para caracteres especiais
    }

    for (uint8_t i = 0; i < 8; ++i) {
        uint8_t line = font[index + i];
        for (uint8_t j = 0; j < 8; ++j) {
            ssd1306_pixel(ssd, x + i, y + j, line & (1 << j));
        }
    }
}

// Função para desenhar uma string no display
void ssd1306_draw_string(ssd1306_t *ssd, const char *str, uint8_t x, uint8_t y) {
    while (*str) {
        ssd1306_draw_char(ssd, *str++, x, y);
        x += 8;
        if (x + 8 >= ssd->width) {
            x = 0;
            y += 8;
        }
        if (y + 8 >= ssd->height) {
            break;
        }
    }
}

// Função para desenhar uma borda usando um caractere
void ssd1306_border_character(ssd1306_t *ssd, uint8_t top, uint8_t left, uint8_t width, uint8_t height, bool value, char character) {
    // Borda superior:
    for (uint8_t x = left; x < left + width; x += WIDTH) {
        ssd1306_draw_char(ssd, character, x, top);
    }
    // Borda inferior:
    uint8_t bottom = top + height - HEIGHT;
    for (uint8_t x = left; x < left + width; x += WIDTH) {
        ssd1306_draw_char(ssd, character, x, bottom);
    }
    // Borda esquerda:
    for (uint8_t y = top; y < top + height; y += HEIGHT) {
        ssd1306_draw_char(ssd, character, left, y);
    }
    // Borda direita:
    uint8_t right = left + width - WIDTH;
    for (uint8_t y = top; y < top + height; y += HEIGHT) {
        ssd1306_draw_char(ssd, character, right, y);
    }
}
