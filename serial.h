#ifndef SERIAL_H
#define SERIAL_H

#include <avr/io.h>
#include <avr/interrupt.h>
#include <stdio.h>

// ATENÇÃO: incluir em um .c só (a main), porque aqui dentro tem a interrupção de recepção

#define SERIAL_BAUD 9600
#define SERIAL_UBRR ((F_CPU / 16 / SERIAL_BAUD) - 1)

//os bytes recebidos ficam guardados aqui pela interrupção até a main ler,
//assim nada se perde enquanto a main está ocupada (LCD, esperando soltar tecla...)
#define SERIAL_TAM_BUFFER 64

static volatile unsigned char serial_buffer[SERIAL_TAM_BUFFER];
static volatile unsigned char serial_inicio = 0;   // próximo byte a ser lido
static volatile unsigned char serial_fim = 0;      // onde entra o próximo byte recebido

ISR(USART_RX_vect) {
    unsigned char dado = UDR0;
    unsigned char proximo = (serial_fim + 1) % SERIAL_TAM_BUFFER;

    if (proximo != serial_inicio) {     // se o buffer encheu, o byte é descartado
        serial_buffer[serial_fim] = dado;
        serial_fim = proximo;
    }
}

//1 se tem byte recebido esperando pra ser lido
static unsigned char serial_disponivel(void) {
    return serial_inicio != serial_fim;
}

//só chamar depois de ver que serial_disponivel() deu 1
static unsigned char serial_ler(void) {
    unsigned char dado = serial_buffer[serial_inicio];
    serial_inicio = (serial_inicio + 1) % SERIAL_TAM_BUFFER;
    return dado;
}

//manda um byte do jeito que ele é (pras mensagens do protocolo)
static void serial_enviar(unsigned char dado) {
    while ((UCSR0A & (1 << UDRE0)) == 0);
    UDR0 = dado;
}

static int serial_putchar(char c, FILE *stream) {
    if (c == '\n') {
        serial_putchar('\r', stream);   // o Monitor Serial espera \r\n
    }
    serial_enviar(c);
    return 0;
}

static FILE serial_saida = FDEV_SETUP_STREAM(serial_putchar, NULL, _FDEV_SETUP_WRITE);

static void serial_init(void) {
    UCSR0A = 0;                                 // o bootloader do Uno deixa a velocidade dobrada (U2X0) ligada: sem isso, 9600 vira 19200
    UBRR0H = (unsigned char)(SERIAL_UBRR >> 8);
    UBRR0L = (unsigned char)SERIAL_UBRR;
    UCSR0B = (1 << RXEN0) | (1 << TXEN0) | (1 << RXCIE0);  // recepção (com interrupção) e transmissão
    UCSR0C = (1 << UCSZ01) | (1 << UCSZ00);     // 8 bits, sem paridade, 1 stop bit
    stdout = &serial_saida;
    sei();                                      // liga as interrupções no geral
}

#endif
