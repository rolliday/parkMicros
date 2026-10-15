//funções para comparação de placa na questão de pessoa especial e cartão vistual 

#include <string.h>

#define QTD_PLACAS 5
#define TAM_PLACA  7

int Placa_Preferencial(const char *Placa) //queria comentar que ja mudei esse nome 3x pq a cada momento chamo diferente ... especial, priritario, preferencial ...
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

//como o valor muda, ele fica fora da função e usei QTD_PLACAS pq o numero é igual
unsigned char saldo_cartao_virtual[QTD_PLACAS] = {
	50,
	20,
	30,
	60,
	20 };

int Desconta_Saldo_Cartao_Virtual(unsigned char pos, unsigned char valor_cobrado)
{
	if (pos >= QTD_PLACAS){
		return 0; 	//caso não exista esse valor, só pra garantir
	}
	if (saldo_cartao_virtual[pos] < valor_cobrado){
		return -1; 	//caso o saldo não seja o suficiente 
	}
	saldo_cartao_virtual[pos] -= valor_cobrado;
	return 1;
}

int main (void){
int indice_cartao;

indice_cartao = Placa_Cartao_Virtual(Placa);

if (indice_cartao != -1)
{
	Desconta_Saldo_Cartao_Virtual(indice, valor_a_pagar); 
}


}
