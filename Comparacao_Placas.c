//funções para comparação de placa na questão de pessoa especial e cartão vistual

#ifndef F_CPU
#define F_CPU 16000000UL
#endif

#include <string.h>

#define QTD_PLACAS 5
#define TAM_PLACA  7

//as listas e o saldo ficam no Dados_Placas_Cartoes.c (um lugar só pros dados)
extern char lista_placas_especiais[QTD_PLACAS][TAM_PLACA + 1];
extern char lista_cartao_virtual[QTD_PLACAS][TAM_PLACA + 1];
//como o valor muda, ele fica fora da função e usei QTD_PLACAS pq o numero é igual
extern unsigned int saldo_cartao_virtual[QTD_PLACAS];     // em centavos

static char eh_letra(char c)  { return c >= 'A' && c <= 'Z'; }
static char eh_numero(char c) { return c >= '0' && c <= '9'; }

//confere se a placa tem um formato válido: Denatran (ABC1234) ou Mercosul (BRA0S17)
//as duas só mudam na 5ª posição, que pode ser número (Denatran) ou letra (Mercosul)
char Placa_Formato_Valido(const char *Placa)
{
	if (strlen(Placa) != TAM_PLACA)
		return 0;
	if (!eh_letra(Placa[0]) || !eh_letra(Placa[1]) || !eh_letra(Placa[2]))
		return 0;	// 3 letras
	if (!eh_numero(Placa[3]))
		return 0;	// 1 número
	if (!eh_numero(Placa[4]) && !eh_letra(Placa[4]))
		return 0;	// número (Denatran) ou letra (Mercosul)
	if (!eh_numero(Placa[5]) || !eh_numero(Placa[6]))
		return 0;	// 2 números
	return 1;
}

char Placa_Preferencial(const char *Placa) //queria comentar que ja mudei esse nome 3x pq a cada momento chamo diferente ... especial, priritario, preferencial ...
{
	for (unsigned char i = 0; i < QTD_PLACAS; i++)
	{
		if (strncmp(Placa, lista_placas_especiais[i], TAM_PLACA) == 0) 
			return 1; 	//retorna que a pessoa é uma pessoa prioritária 
	}
	return 0;	// retorna que a pss não é da lista de prioritária 
}

signed char Placa_Cartao_Virtual (const char *Placa)
{
	for (unsigned char i = 0; i < QTD_PLACAS; i++)
	{
		if (strncmp(Placa, lista_cartao_virtual[i], TAM_PLACA) == 0) 
			return i; 	//pra retornar a posição do cartçao que tem que ser cobrado 
	}
	return -1; 	// pro retorno ser invalido e saber que o cartão não ta na lista 
}

signed char Desconta_Saldo_Cartao_Virtual(unsigned char pos, int valor_cobrado)
{
	//o saldo também é em centavos, então desconta o valor exato (antes cortava os centavos)
	
	if (pos >= QTD_PLACAS){
		return 0; 	//caso não exista esse valor, só pra garantir
	}
	if (valor_cobrado < 0 || saldo_cartao_virtual[pos] < (unsigned int)valor_cobrado){
		return -1; 	//caso o saldo não seja o suficiente 
	}
	saldo_cartao_virtual[pos] -= valor_cobrado;
	return 1;
}
