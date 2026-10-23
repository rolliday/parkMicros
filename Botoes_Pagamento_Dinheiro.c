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
#define PINO_50CENT     PB3     // pino 11 do Arduino (o 13 tem o LED da placa e não funcionou)

#define VALOR_1REAL     100     
#define VALOR_50CENT    50      

#define PRESSIONADO     0       // pull-up: botão pressionado lê 0

//configura os botões
void botoes_init(void) {
    BOTAO_DDR  &= ~((1 << PINO_1REAL) | (1 << PINO_50CENT));   // entradas
    BOTAO_PORT |=  ((1 << PINO_1REAL) | (1 << PINO_50CENT));   // pull-up ligado
    Timer0_Iniciar();                                           // o Debouncer usa o Delay1ms (Timer0)
}

//lê os botões uma vez só e devolve a moeda que acabou de entrar (VALOR_1REAL, VALOR_50CENT) ou 0
//não fica presa esperando, então quem chama pode mostrar no LCD quanto falta a cada moeda
int botoes_ler_moeda(void)
{
    static char botao_1real  = 0;   // 1 = botão ainda pressionado
    static char botao_50cent = 0;
    int moeda = 0;

    if (Debouncer(PINO_1REAL) == PRESSIONADO) {
        if (!botao_1real) {            // início do aperto -> conta uma vez
            botao_1real = 1;
            moeda += VALOR_1REAL;
          }
        } else {
        botao_1real = 0;               // soltou, pode contar de novo
        }
    if (Debouncer(PINO_50CENT) == PRESSIONADO) {
        if (!botao_50cent) {
            botao_50cent = 1;
            moeda += VALOR_50CENT;
            }
        } else {
            botao_50cent = 0;
        }
    return moeda;
}

int botoes_pagamento_em_dinheiro(int valor_a_pagar)
{
    int valor_pago = 0;

    while (valor_pago < valor_a_pagar){
        valor_pago += botoes_ler_moeda();
    }
    //valor_pago = valor_pago/100;
    //se quiser usar em real, mas sinceramente acho trabalho demais já que usei tudo em centavos
    return valor_pago;
}
