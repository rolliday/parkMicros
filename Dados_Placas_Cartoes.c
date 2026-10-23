#ifndef F_CPU
#define F_CPU 16000000UL
#endif

//placas de pessoas idosas e pcds
char lista_placas_especiais[5][8] = {
	"IOS0098",
	"PEI1Z92",
	"IDO0192",
	"IDS0090",
	"PDE3400" };

//lista do cartão virtual, que eu jurei que se chamava cartão azul 
char lista_cartao_virtual[5][8] = {
	"WCC9850",
	"PIN0C20",
	"CAR0230",
	"VIR7960",
	"PAR8320" };

//saldo em centavos, igual aos valores cobrados (5000 = R$ 50,00)
unsigned int saldo_cartao_virtual[5] = {
	5000,
	2000,
	3000,
	6000,
	2000 };
