// minha ideia para a lógica de pagamento em dinheiro 
//tem que ver melhor esse sensor com 

#include <avr/io.h>
#include <util/delay.h>
#include <stdint.h>

#define F_CPU 16000000UL
#define LIMIAR          80      // Variação mínima para o ldr contar 

#define CANAL_1REAL     0       // Entrada A0
#define CANAL_50CENT    1       // Entrada A1

#define VALOR_1REAL     100     // centavos
#define VALOR_50CENT    50      // centavos

static int ref_1real, ref_50cent;

void adc_init(void){
    ADMUX  = (1 << REFS0);
    ADCSRA = (1 << ADEN) | (1 << ADPS2) | (1 << ADPS1) | (1 << ADPS0);
}

int adc_ler(char canal){
    ADMUX = (ADMUX & 0xF0) | (canal & 0x0F);
    ADCSRA |= (1 << ADSC);
    while (ADCSRA & (1 << ADSC));
    return ADC;
}
 //valor de referencia para a entrada do ldr 
void calibrar_ldrs(void){
    long soma_1 = 0, soma_50 = 0;
  
    for (char i = 0; i < 32; i++) {
        soma_1  += adc_ler(CANAL_1REAL);
        soma_50 += adc_ler(CANAL_50CENT);
    }
    ref_1real  = soma_1 / 32;
    ref_50cent = soma_50 / 32;
}

// diferença absoluta entre a entrada atual e a ref, pra não ter erro de leitura 
static char sombra(int leitura, int ref){
    int dif = (leitura > ref) ? (leitura - ref) : (ref - leitura);
    return dif > LIMIAR;
}

int pagamento_em_dinheiro(int valor_a_pagar)
{
    static char sensor_1real  = 0;   // 1 = objeto ainda na frente do LDR
    static char sensor_50cent = 0;
    int valor_pago = 0;

    while (valor_pago < valor_a_pagar) {
        // sensor de 1 real 
        if (sombra(adc_ler(CANAL_1REAL), ref_1real)) {
            if (!sensor_1real) {            // início da passagem -> conta uma vez
                sensor_1real = 1;           // a variavel muda de estado para evitar que a mesma moeda seja contada varias vezes 
                valor_pago += VALOR_1REAL;
            }
        } else {
            sensor_1real = 0;               // liberou, pode contar de novo
        }
        
        // sensor para 50 centavos 
        if (sombra(adc_ler(CANAL_50CENT), ref_50cent)) {
            if (!sensor_50cent) {
                sensor_50cent = 1;
                valor_pago += VALOR_50CENT;
            }
        } else {
            sensor_50cent = 0;
        }
    }
    return valor_pago;         // em centavos!!!!
}

int main(void)
{
    int valor_a_pagar =0; 
    int valor_pago = 0;
    int troco = 0; 
    
    adc_init();
    calibrar_ldrs();

    while (1)
    {
        //atualiza o valor a pagar, em centavos já que quando a gente manda pro sistema do sor tb é em centavos 
        valor_pago = pagamento_em_dinheiro(valor_a_pagar);
        if (valor_a_pagar < foi_pago) {
            troco = valor_pago - valor_a_pagar;  
            //talvez mostrar no LCD "Seu troco é :  "
            // adicionei isso, pois esqueci antes e caso a gente use ta pronto 
        }
    }
}
