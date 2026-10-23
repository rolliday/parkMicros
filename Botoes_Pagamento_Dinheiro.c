//triste em jogar meu Sensor_LDR fora ...

#ifndef F_CPU
#define F_CPU 16000000UL    // tem que vir antes do <util/delay.h>, senão o _delay_ms conta errado
#endif

#include <avr/io.h>
#include <util/delay.h>

//funções do Teclado 4x3.c que este arquivo usa
//o Debouncer de lá lê o PINB, que é a mesma porta dos botões (PB4 e PB5)
void Timer0_Iniciar(void);
unsigned char Debouncer(unsigned char pino);

#define BOTAO_PIN       PINB    // registrador de leitura dos botões
#define BOTAO_PORT      PORTB
#define BOTAO_DDR       DDRB

#define PINO_1REAL      PB4     // pino 12 do Arduino
#define PINO_50CENT     PB5     // pino 13 do Arduino

#define VALOR_1REAL     100     
#define VALOR_50CENT    50      

#define PRESSIONADO     0       // pull-up: botão pressionado lê 0

//configura os botões
void botoes_init(void) {
    BOTAO_DDR  &= ~((1 << PINO_1REAL) | (1 << PINO_50CENT));   // entradas
    BOTAO_PORT |=  ((1 << PINO_1REAL) | (1 << PINO_50CENT));   // pull-up ligado
    Timer0_Iniciar();                                           // o Debouncer usa o Delay1ms (Timer0)
}

int botoes_pagamento_em_dinheiro(int valor_a_pagar)
{
    static char botao_1real  = 0;   // 1 = botão ainda pressionado
    static char botao_50cent = 0;
    int valor_pago = 0;

    while (valor_pago < valor_a_pagar){
        if (Debouncer(PINO_1REAL) == PRESSIONADO) {
            if (!botao_1real) {            // início do aperto -> conta uma vez
                botao_1real = 1;
                valor_pago += VALOR_1REAL;
              }
            } else {
            botao_1real = 0;               // soltou, pode contar de novo
            } 
        if (Debouncer(PINO_50CENT) == PRESSIONADO) {
            if (!botao_50cent) {
                botao_50cent = 1;
                valor_pago += VALOR_50CENT;
                }
            } else {
                botao_50cent = 0;
            }
    }
    //valor_pago = valor_pago/100; 
    //se quiser usar em real, mas sinceramente acho trabalho demais já que usei tudo em centavos
    return valor_pago;
}
