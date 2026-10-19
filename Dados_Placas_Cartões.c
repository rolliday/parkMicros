#ifndef F_CPU
#define F_CPU 16000000UL
#endif

#include "serial.h"

//placas de pessoas idosas e pcds
char lista_placas_especiais[5][8] = {
	"IOS0098",
	"PEI1Z92",
	"IDO0192",
	"IDS0090",
	"PDE3400" };

//lista do cartão virtual, que eu judei que se chamava catão azul 
char lista_cartao_virtual[5][8] = {
	"WCC9850",
	"PIN0C20",
	"CAR0230",
	"VIR7960",
	"PAR8320" };

unsigned char saldo_cartao_virtual[5] = {
	50,
	20,
	30,
	60,
	20 };

// main de teste: imprime as listas no Monitor Serial (9600) pra conferir os dados
// compilar com -DSEM_MAIN pra usar só os dados junto com outro arquivo
#ifndef SEM_MAIN
int main(void) {
	serial_init();

	printf("Placas especiais (idoso/PCD):\n");
	for (unsigned char i = 0; i < 5; i++) {
		printf("  %s\n", lista_placas_especiais[i]);
	}

	printf("Cartao virtual (placa: saldo em reais):\n");
	for (unsigned char i = 0; i < 5; i++) {
		printf("  %s: %d\n", lista_cartao_virtual[i], saldo_cartao_virtual[i]);
	}

	while (1);	// no microcontrolador o main não pode terminar
}
#endif
