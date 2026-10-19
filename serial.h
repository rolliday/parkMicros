#ifndef SERIAL_H
#define SERIAL_H

#include <avr/io.h>
#include <stdio.h>

#define SERIAL_BAUD 9600
#define SERIAL_UBRR ((F_CPU / 16 / SERIAL_BAUD) - 1)

static int serial_putchar(char c, FILE *stream) {
    if (c == '\n') {
        serial_putchar('\r', stream);   // o Monitor Serial espera \r\n
    }
    while ((UCSR0A & (1 << UDRE0)) == 0);
    UDR0 = c;
    return 0;
}

static FILE serial_saida = FDEV_SETUP_STREAM(serial_putchar, NULL, _FDEV_SETUP_WRITE);

static void serial_init(void) {
    UBRR0H = (unsigned char)(SERIAL_UBRR >> 8);
    UBRR0L = (unsigned char)SERIAL_UBRR;
    UCSR0B = (1 << TXEN0);                      // só transmissão
    UCSR0C = (1 << UCSZ01) | (1 << UCSZ00);     // 8 bits, sem paridade, 1 stop bit
    stdout = &serial_saida;
}

#endif
