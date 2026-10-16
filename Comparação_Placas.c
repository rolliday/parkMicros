//funções para comparação de placa na questão de pessoa especial e cartão vistual 

#include <string.h>

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

	for (char i = 0; i < QTD_PLACAS; i++)
	{
		if (strncmp(Placa, lista_placas_especiais[i], TAM_PLACA) == 0) 
			return 1; 	//retorna que a pessoa é uma pessoa prioritária 
	}
	return 0;	// retorna que a pss não é da lista de prioritária 
}

char Placa_Cartao_Virtual (const char *Placa)
{
	static const char lista_cartao_virtual[QTD_PLACAS][TAM_PLACA + 1] = {
	"WCC9850",
	"PIN0C20",
	"CAR0230",
	"VIR7960",
	"PAR8320" };

	for (char i = 0; i < QTD_PLACAS; i++)
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

char Desconta_Saldo_Cartao_Virtual(unsigned char pos, int valor_cobrado)
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

int main (void) {
	char indice_cartao = 0;
	int valor_a_pagar = 0; 
	
	indice_cartao = Placa_Cartao_Virtual(Placa);
	
	if (indice_cartao != -1)
	{
		indice_cartao = Desconta_Saldo_Cartao_Virtual(indice_cartao, valor_a_pagar);
		if (indice_cartao == 1){
			//confirmação de pagamento 
		}
		if (indice_cartao == -1){
			//Lcd "Saldo Insuficiente"
		}else{
				//FUDEO, pq não é prar dar errado 
			}
	}
}
