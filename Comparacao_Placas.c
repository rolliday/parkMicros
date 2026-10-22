//funções para comparação de placa na questão de pessoa especial e cartão vistual

#ifndef F_CPU
#define F_CPU 16000000UL
#endif

#include <string.h>
#include "serial.h"

#define QTD_PLACAS 5
#define TAM_PLACA  7

char Placa_Preferencial(const char *Placa) //queria comentar que ja mudei esse nome 3x pq a cada momento chamo diferente ... especial, priritario, preferencial ...
{
	static const char lista_placas_especiais[QTD_PLACAS][TAM_PLACA + 1] = {
		"IOS0098",
		"PEI1Z92",
		"IDO0192",
		"IDS0090",
		"PDE3400" };

	for (unsigned char i = 0; i < QTD_PLACAS; i++)
	{
		if (strncmp(Placa, lista_placas_especiais[i], TAM_PLACA) == 0) 
			return 1; 	//retorna que a pessoa é uma pessoa prioritária 
	}
	return 0;	// retorna que a pss não é da lista de prioritária 
}

signed char Placa_Cartao_Virtual (const char *Placa)
{
	static const char lista_cartao_virtual[QTD_PLACAS][TAM_PLACA + 1] = {
	"WCC9850",
	"PIN0C20",
	"CAR0230",
	"VIR7960",
	"PAR8320" };

	for (unsigned char i = 0; i < QTD_PLACAS; i++)
	{
		if (strncmp(Placa, lista_cartao_virtual[i], TAM_PLACA) == 0) 
			return i; 	//pra retornar a posição do cartçao que tem que ser cobrado 
	}
	return -1; 	// pro retorno ser invalido e saber que o cartão não ta na lista 
}

//como o valor muda, ele fica fora da função e usei QTD_PLACAS pq o numero é igual
unsigned char saldo_cartao_virtual[QTD_PLACAS] = {
	50,
	20,
	30,
	60,
	20 };

signed char Desconta_Saldo_Cartao_Virtual(unsigned char pos, int valor_cobrado)
{
	valor_cobrado = valor_cobrado/100; 	//passar de centavos para real 
	
	if (pos >= QTD_PLACAS){
		return 0; 	//caso não exista esse valor, só pra garantir
	}
	if (saldo_cartao_virtual[pos] < valor_cobrado){
		return -1; 	//caso o saldo não seja o suficiente 
	}
	saldo_cartao_virtual[pos] -= valor_cobrado;
	return 1;
}

// main de teste: mostra no Monitor Serial (9600) o resultado de cada placa
// compilar com -DSEM_MAIN pra usar só as funções junto com outro arquivo
#ifndef SEM_MAIN
int main (void) {
	static const char *placas_teste[] = { "IOS0098", "WCC9850", "PIN0C20", "ABC1234" };
	signed char indice_cartao = 0;	// signed: tem que caber o -1 (no Atmel Studio o char é unsigned)
	signed char resultado = 0;
	int valor_a_pagar = 2500;	// R$ 25,00 só pra testar (o PIN0C20 tem 20 de saldo, então tem que dar insuficiente)

	serial_init();

	for (unsigned char p = 0; p < 4; p++)
	{
		const char *Placa = placas_teste[p];
		printf("Placa %s: ", Placa);

		if (Placa_Preferencial(Placa)){
			printf("preferencial, ");
		}

		indice_cartao = Placa_Cartao_Virtual(Placa);

		if (indice_cartao != -1)
		{
			resultado = Desconta_Saldo_Cartao_Virtual(indice_cartao, valor_a_pagar);
			if (resultado == 1){
				//confirmação de pagamento
				printf("cartao virtual pago, saldo agora %d\n", saldo_cartao_virtual[(unsigned char)indice_cartao]);
			}else if (resultado == -1){
				//Lcd "Saldo Insuficiente"
				printf("cartao virtual com saldo insuficiente\n");
			}else{
				//FUDEO, pq não é prar dar errado
				printf("erro no cartao virtual\n");
			}
		}else{
			printf("sem cartao virtual\n");
		}
	}

	while (1);	// no microcontrolador o main não pode terminar
}
#endif
