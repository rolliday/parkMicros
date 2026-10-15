//funções para comparação de placa na questão de pessoa especial e cartão vistual 

#include <string.h>
#include <avr/pgmspace.h>

#define QTD_PLACAS 5
#define TAM_PLACA  7

int Placa_Especial(const char *Placa)
{
	static const char lista_placas_especiais[QTD_PLACAS][TAM_PLACA + 1] PROGMEM = {
		"IOS0098",
		"PEI1Z92",
		"IDO0192",
		"IDS0090",
		"PDE3400" };

	for (int i = 0; i < QTD_PLACAS; i++)
	{
		if (strncmp_P(Placa, lista_placas_especiais[i], TAM_PLACA) == 0)   //strncmp pois é um testo pré fixado 
			return 1;
	}
	return 0;
}

int Placa_Cartao_Virtual (const char *Placa)
{
	static const 

	for (int i = 0; i < QTD_PLACAS; i++)
	{
		if (strncmp_P(Placa, lista_placas_especiais[i], TAM_PLACA) == 0)   //strncmp pois é um testo pré fixado 
			return 1;
	}
	return 0;
}
