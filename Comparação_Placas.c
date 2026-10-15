//funções para comparação de placa na questão de pessoa especial e cartão vistual 

#include <string.h>

#define QTD_PLACAS 5
#define TAM_PLACA  7

int Placa_Especial(const char *Placa)
{
	static const char lista_placas_especiais[QTD_PLACAS][TAM_PLACA + 1] = {
		"IOS0098",
		"PEI1Z92",
		"IDO0192",
		"IDS0090",
		"PDE3400" };

	for (int i = 0; i < QTD_PLACAS; i++)
	{
		if (strncmp(Placa, lista_placas_especiais[i], TAM_PLACA) == 0) 
			return 1; 	//retorna que a pessoa é uma pessoa prioritária 
	}
	return 0;	// retorna que a pss não é da lista de prioritária 
}

int Placa_Cartao_Virtual (const char *Placa)
{
	static const char lista_cartao_virtual[QTD_PLACAS][TAM_PLACA + 1] = {
	"WCC9850",
	"PIN0C20",
	"CAR0230",
	"VIR7960",
	"PAR8320" };

	for (int i = 0; i < QTD_PLACAS; i++)
	{
		if (strncmp(Placa, lista_cartao_virtual[i], TAM_PLACA) == 0) 
			return i; 	//pra retornar a posição do cartçao que tem que ser cobrado 
	}
	return -1; 	// pro retorno ser invalido e saber que o cartão não ta na lista 
}
