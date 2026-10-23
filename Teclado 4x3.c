/* Pinagem para o Arduino Uno, pinos 1 a 7 do teclado ligados nos pinos 4 a 10 do Arduino
   (a do lab 6 usava PORTA e PORTH, que só existem no Mega)
    Linhas (saídas):
      Linha 0 -> PD4 (pino 4)  <- pino 1 do teclado
      Linha 1 -> PD5 (pino 5)  <- pino 2 do teclado
      Linha 2 -> PD6 (pino 6)  <- pino 3 do teclado
      Linha 3 -> PD7 (pino 7)  <- pino 4 do teclado
    Colunas (entradas com pull-up):
      Coluna 0 -> PB0 (pino 8)  <- pino 5 do teclado
      Coluna 1 -> PB1 (pino 9)  <- pino 6 do teclado
      Coluna 2 -> PB2 (pino 10) <- pino 7 do teclado
    se as teclas saírem trocadas, é só ajustar o mapa_teclas (alguns teclados têm outra ordem nos pinos)
 */

#ifndef F_CPU
#define F_CPU 16000000UL    // tem que vir antes do <util/delay.h>, senão o _delay_us conta errado
#endif

#include <avr/io.h>
#include <util/delay.h>
#include <string.h>

// as funções de tempo e o Debouncer daqui também são usadas pelo Teste_Teclado_LCD.c,
// pelo Botoes_Pagamento_Dinheiro.c e pela main (cada uma existe só aqui)

//funções do Teste_Teclado_LCD.c que a leitura da placa usa pra mostrar o que está sendo digitado
void lcd_set_cursor(unsigned char col, unsigned char linha);
void lcd_putc(char c);

#define PLACA_TAM          7        // ABC1234 ou BRA0S17
#define MULTITAP_ESPERA_MS 1000     // 1 s sem apertar confirma a letra

#define BOUNCE 10
#define T0_RELOAD_1MS  6
#define T0_PRESCALE_64 ((1 << CS01) | (1 << CS00))

#define NUM_LINHAS  4
#define NUM_COLUNAS 3
#define SEM_TECLA   0x00
#define INVALIDO    2  // não é um valor de estado real 

#define COLUNA_DDR     DDRB
#define COLUNA_PORT    PORTB
#define COLUNA_PIN     PINB
#define COLUNA_BASE    PB0
#define COLUNA_MASCARA ((1 << PB0) | (1 << PB1) | (1 << PB2))

#define LINHA_DDR      DDRD
#define LINHA_PORT     PORTD
#define LINHA_BASE     PD4
#define LINHA_MASCARA  ((1 << PD4) | (1 << PD5) | (1 << PD6) | (1 << PD7))

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
	TIFR0 = (1 << TOV0);	// limpa a flag antes, senão a primeira espera pode sair na hora
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

//multitap (padrão de telefone): apertar a mesma tecla de novo passa pra próxima letra
static const char *const letras_tecla[10] = {
	"0",        // tecla 0
	"1",        // tecla 1
	"ABC2",     // tecla 2
	"DEF3",     // tecla 3
	"GHI4",     // tecla 4
	"JKL5",     // tecla 5
	"MNO6",     // tecla 6
	"PQRS7",    // tecla 7
	"TUV8",     // tecla 8
	"WXYZ9"     // tecla 9
};

/*lê a placa com o multitap e vai mostrando no LCD a partir de (col, linha)
  - a mesma tecla de novo troca a letra (2 -> A, B, C, 2, A...)
  - 1 s sem apertar ou outra tecla confirma a letra
  - # apaga a última letra, * termina (mesmo com menos de 7; quem chama confere o formato)
  placa precisa ter espaço pra 8 (7 + '\0') */
void Teclado_Ler_Placa(char *placa, unsigned char col, unsigned char linha) {
	unsigned char n = 0;                    // letras já confirmadas
	unsigned char tecla_atual = SEM_TECLA;  // tecla que está sendo alternada (SEM_TECLA = nenhuma)
	unsigned char indice = 0;               // qual letra da tecla_atual está aparecendo
	unsigned int espera = 0;                // ms desde o último aperto
	unsigned char tecla;
	const char *letras;

	placa[0] = '\0';

	while (1) {
		tecla = Teclado_Ler();

		if (tecla == SEM_TECLA) {
			if (tecla_atual != SEM_TECLA) {     // tem letra esperando confirmação
				Delay1ms();
				espera++;
				if (espera >= MULTITAP_ESPERA_MS) {
					n++;                        // confirmou por tempo
					tecla_atual = SEM_TECLA;
				}
			}
			continue;
		}

		if (tecla == '#') {                     // apaga
			if (tecla_atual == SEM_TECLA) {     // sem letra pendente: apaga a última confirmada
				if (n == 0) {
					continue;
				}
				n--;
			}
			tecla_atual = SEM_TECLA;
			placa[n] = '\0';
			lcd_set_cursor(col + n, linha);
			lcd_putc(' ');
			continue;
		}

		if (tecla == '*') {                     // termina
			if (tecla_atual != SEM_TECLA) {     // confirma a letra pendente
				n++;
			}
			placa[n] = '\0';
			return;                             // com menos de 7, a main mostra "Placa invalida!"
		}

		//tecla de 0 a 9
		if (tecla == tecla_atual) {             // mesma tecla: próxima letra dela
			letras = letras_tecla[tecla - '0'];
			indice = (indice + 1) % strlen(letras);
		} else {
			if (tecla_atual != SEM_TECLA) {     // outra tecla confirma a letra pendente
				n++;
			}
			if (n >= PLACA_TAM) {               // já tem 7: não escreve mais
				tecla_atual = SEM_TECLA;
				continue;
			}
			tecla_atual = tecla;
			indice = 0;
			letras = letras_tecla[tecla - '0'];
		}

		placa[n] = letras[indice];
		placa[n + 1] = '\0';
		lcd_set_cursor(col + n, linha);
		lcd_putc(placa[n]);
		espera = 0;
	}
}
