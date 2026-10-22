/* LED de alerta: pisca a 1 Hz quando o pagamento não foi feito (passou o tempo + 3 min de tolerância)
    LED -> PD3 (pino 3), com resistor de 220 ohms até o GND
   o pisca é feito pelo Timer1 com interrupção, então o resto do programa continua rodando
   enquanto o LED pisca (não fica preso num delay)
 */

#include <avr/io.h>
#include <avr/interrupt.h>

#define LED_DDR         DDRD
#define LED_PORT        PORTD
#define LED_PINO        PD3

//1 Hz = 0,5 s aceso + 0,5 s apagado, então o Timer1 inverte o LED a cada 0,5 s
//16 MHz / 256 = 62500 contagens por segundo -> 0,5 s = 31250 contagens
#define T1_PRESCALE_256 (1 << CS12)
#define T1_OCR_500MS    31249

static volatile unsigned char led_piscando = 0;    // volatile: é mudada fora da interrupção

ISR(TIMER1_COMPA_vect) {
    if (led_piscando) {
        LED_PORT ^= (1 << LED_PINO);    // inverte o LED
    }
}

void LED_Alerta_Iniciar(void) {
    LED_DDR  |=  (1 << LED_PINO);       // saída
    LED_PORT &= ~(1 << LED_PINO);       // começa apagado

    TCCR1A = 0x00;
    TCCR1B = (1 << WGM12) | T1_PRESCALE_256;   // modo CTC: zera quando chega no OCR1A
    OCR1A  = T1_OCR_500MS;
    TIMSK1 = (1 << OCIE1A);             // liga a interrupção de comparação
    sei();                              // liga as interrupções no geral
}

//chamar quando o tempo acabar sem pagamento
void LED_Alerta_Ligar(void) {
    if (!led_piscando) {
        TCNT1 = 0;                      // começa o 1o meio segundo do zero
        LED_PORT |= (1 << LED_PINO);    // já acende na hora
        led_piscando = 1;
    }
}

//chamar quando o pagamento for feito
void LED_Alerta_Desligar(void) {
    led_piscando = 0;
    LED_PORT &= ~(1 << LED_PINO);
}
