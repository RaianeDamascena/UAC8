# UAC8

## Objetivo

Este projeto tem como objetivo consolidar os conceitos sobre o uso de conversores analógico-digitais (ADC) no RP2040 e explorar as funcionalidades da placa de desenvolvimento BitDogLab.

## Descrição do Projeto

O joystick fornecerá valores analógicos correspondentes aos eixos X e Y, que serão utilizados para os seguintes controles:

Controle dos LEDs RGB:

LED Azul: Brilho ajustado conforme o valor do eixo Y.

LED Vermelho: Brilho ajustado conforme o valor do eixo X.

## Controle do Display SSD1306:

Um quadrado de 8x8 pixels será exibido no display SSD1306.

O quadrado se moverá proporcionalmente aos valores capturados pelo joystick.


## Funções adicionais:

Botão do Joystick (GPIO 22):

Alterna o estado do LED Verde a cada acionamento.


Botão A (GPIO 5):

Ativa ou desativa os LEDs controlados via PWM a cada acionamento.


## Componentes Utilizados

Os seguintes componentes serão utilizados e interconectados à placa BitDogLab:

LED RGB, conectado às GPIOs (11, 12 e 13).

Botão do Joystick, conectado à GPIO 22.

Joystick, conectado às GPIOs 26 e 27.

Botão A, conectado à GPIO 5.

Display SSD1306, conectado via I2C (GPIO 14 e GPIO 15).


## Link do video funcionando na placa bitdoglab

https://drive.google.com/file/d/1aU8eBNUxFIy38B7hERJ2j7Ssnu8-t5kA/view?usp=sharing


## Desenvolvedor

Raiane Damascena📧 
E-mail: rai.paixao2012@gmail.com

