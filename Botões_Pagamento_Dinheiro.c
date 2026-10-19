//triste em jogar meu Sensor_LDR fora ...

#ifndef F_CPU
#define F_CPU 16000000UL    // tem que vir antes do <util/delay.h>, senão o _delay_ms conta errado
#endif

#include <avr/io.h>
#include <util/delay.h>
#include "serial.h"

#define COLUNA_PIN      PIND    // registrador de leitura dos botões
#define COLUNA_PORT     PORTD
#define COLUNA_DDR      DDRD

#define PINO_1REAL      2       // PD2
#define PINO_50CENT     3       // PD3

#define VALOR_1REAL     100     
#define VALOR_50CENT    50      

#define BOUNCE          10
#define INVALIDO        2

#define PRESSIONADO     0       // pull-up: botão pressionado lê 0

void Delay1ms(void) {
    _delay_ms(1);
}

unsigned char Debouncer(unsigned char pino) {
    unsigned char count = 0;
    unsigned char anterior = INVALIDO;
    unsigned char atual;

    while (1) {
        Delay1ms();
        atual = (COLUNA_PIN >> pino) & 0x01;
        if (atual == anterior) {
            count++;
        } else {
            count = 0;
        }
        if (count == BOUNCE) {
            return atual;
        }
        anterior = atual;
    }
}

//configura os botões 
void botoes_init(void) {
    COLUNA_DDR  &= ~((1 << PINO_1REAL) | (1 << PINO_50CENT));   // entradas
    COLUNA_PORT |=  ((1 << PINO_1REAL) | (1 << PINO_50CENT));   // pull-up ligado
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

// main de teste: botão de 1 real no pino 2 (PD2) e de 50 centavos no pino 3 (PD3), ligados no GND
// compilar com -DSEM_MAIN pra usar só as funções junto com outro arquivo
#ifndef SEM_MAIN
int main(void)
{
    //po assumo que não sei se mexo nesses aq, já que os valores vao ser tipo 100, 150, ...
    int valor_pago = 0;
    int valor_a_pagar = 250;    // R$ 2,50 só pra testar
    int troco = 0;
    botoes_init();
    serial_init();

    while (1)
    {
        printf("Valor a pagar: %d centavos\n", valor_a_pagar);
        valor_pago = botoes_pagamento_em_dinheiro(valor_a_pagar);   //passar o valor em centavos
        troco = 0;
        if (valor_pago > valor_a_pagar) {
          troco = valor_pago - valor_a_pagar;
          //nao sei o que fazer com isso ...
          //talvez mostrar no LCD "Seu troco é :  "
        }
        printf("Pago: %d centavos, troco: %d centavos\n", valor_pago, troco);
    }
}
#endif
