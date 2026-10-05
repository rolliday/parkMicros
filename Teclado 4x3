/* Usei a pinagem com base no lab 6 
    Colunas (entradas com pull-up):
      Coluna 0 -> PH3 (pino 6)
      Coluna 1 -> PH4 (pino 7)
      Coluna 2 -> PH5 (pino 8)
    Linhas (saídas):
      Linha 0 -> PA0 (pino 22)
      Linha 1 -> PA1 (pino 23)
      Linha 2 -> PA2 (pino 24)
      Linha 3 -> PA3 (pino 25)
 */

#include <avr/io.h>
#include <util/delay.h>

#define F_CPU 16000000UL
#define BOUNCE 10  
#define T0_RELOAD_1MS  6
#define T0_PRESCALE_64 ((1 << CS01) | (1 << CS00))

#define NUM_LINHAS  4
#define NUM_COLUNAS 3
#define SEM_TECLA   0x00
#define INVALIDO    2  // não é um valor de estado real 

#define COLUNA_DDR     DDRH
#define COLUNA_PORT    PORTH
#define COLUNA_PIN     PINH
#define COLUNA_BASE    PH3           
#define COLUNA_MASCARA ((1 << PH3) | (1 << PH4) | (1 << PH5))

#define LINHA_DDR      DDRA
#define LINHA_PORT     PORTA
#define LINHA_BASE     PA0          
#define LINHA_MASCARA  ((1 << PA0) | (1 << PA1) | (1 << PA2) | (1 << PA3))

static unsigned char mapa_teclas[NUM_LINHAS][NUM_COLUNAS] = 
{
	{ '1', '2', '3' },
	{ '4', '5', '6' },
	{ '7', '8', '9' },
	{ '*', '0', '#' }
};


void Timer0_Iniciar(void) {
	TCCR0A = 0x00;
	TCCR0B = T0_PRESCALE_64;
}

void Delay1ms(void) {
	TCNT0 = T0_RELOAD_1MS;
	while ((TIFR0 & (1 << TOV0)) == 0);
	TIFR0 = (1 << TOV0);
}

void DelayNms(unsigned int n) {
	while (n--) {
		Delay1ms();
	}
}

/*EU TENTEI usar o debauce do lab 6 como modelo na aplicação da leitura do teclado completo*/
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

unsigned char Teclado_Configurar(unsigned char linha, unsigned char coluna, unsigned char valor) 
{
	if (linha >= NUM_LINHAS || coluna >= NUM_COLUNAS) {
		return 0;
	}
	mapa_teclas[linha][coluna] = valor;
	return 1;
}

void Teclado_Iniciar(void) {
	COLUNA_DDR  &= ~COLUNA_MASCARA;
	COLUNA_PORT |=  COLUNA_MASCARA;

	LINHA_DDR  |= LINHA_MASCARA;
	LINHA_PORT |= LINHA_MASCARA;

	Timer0_Iniciar();
}

/*varre as linhas, se acha a condição 0, valida com o debauce, espera soltar e retorna a tecla 
e se nenhuma tecla for precionada retorna SEM_TECLA */
unsigned char Teclado_Ler(void) {
	unsigned char linha, coluna, pino;
	unsigned char tecla = SEM_TECLA;

	for (linha = 0; linha < NUM_LINHAS; linha++) {
		LINHA_PORT &= ~(1 << (LINHA_BASE + linha));   //ativa a linha 
		_delay_us(5);                                 // estabiliza o sinal

		for (coluna = 0; coluna < NUM_COLUNAS; coluna++) {
			pino = COLUNA_BASE + coluna;

			if ((COLUNA_PIN & (1 << pino)) == 0) {
				if (Debouncer(pino) == 0) {
					tecla = mapa_teclas[linha][coluna];
					while ((COLUNA_PIN & (1 << pino)) == 0);   // espera soltar
					DelayNms(BOUNCE);
					break;
				}
			}
		}

		LINHA_PORT |= (1 << (LINHA_BASE + linha));    // desativa a linha
		if (tecla != SEM_TECLA) {
			break;
		}
	}
	return tecla;
}
