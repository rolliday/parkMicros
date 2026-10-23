//minha versão da main
#ifndef F_CPU
#define F_CPU 16000000UL    // tem que vir antes do <util/delay.h>, senão o _delay_ms conta errado
#endif

#include <avr/io.h>
#include <util/delay.h>
#include <string.h>
#include "serial.h"         // recepção e envio pela serial (só a main inclui, tem a interrupção)

#define SEM_TECLA   0x00    // tem que ser o mesmo valor do Teclado 4x3.c

#define SERIAL_ESPERA_MS 100    // quanto tempo espera cada byte de uma mensagem antes de desistir

//funções dos outros arquivos (todos os .c ficam como Compile no projeto)
void Timer0_Iniciar(void);                  // Teclado 4x3.c
void Delay1ms(void);                        // Teclado 4x3.c
void DelayNms(unsigned int n);              // Teclado 4x3.c
void Teclado_Iniciar(void);                 // Teclado 4x3.c
unsigned char Teclado_Ler(void);            // Teclado 4x3.c
void Teclado_Ler_Placa(char *placa, unsigned char col, unsigned char linha);   // Teclado 4x3.c
char Placa_Formato_Valido(const char *Placa);   // Comparacao_Placas.c
char Placa_Preferencial(const char *Placa);     // Comparacao_Placas.c
void lcd_init(void);                        // Teste_Teclado_LCD.c
void lcd_clear(void);                       // Teste_Teclado_LCD.c
void lcd_set_cursor(unsigned char col, unsigned char linha);   // Teste_Teclado_LCD.c
void lcd_putc(char c);                      // Teste_Teclado_LCD.c
void lcd_print(const char *s);              // Teste_Teclado_LCD.c
void LED_Alerta_Iniciar(void);              // LED_Alerta.c (Timer1)
unsigned int Relogio_Meios_Segundos(void);  // LED_Alerta.c (Timer1)
unsigned char Calendario_Flag(unsigned char dia, unsigned char mes, unsigned int ano);   // calendario.c
unsigned char Dias_Do_Mes(unsigned char mes, unsigned int ano);                          // calendario.c

//data e hora: acertadas pelo sistema do professor ('S' 'H') e depois contadas pelo próprio Arduino
static unsigned char dia, mes, hora, minuto, segundo;
static unsigned int ano;
static unsigned char hora_recebida = 0;     // 0 enquanto não chegou nenhuma 'S' 'H'
static unsigned char relogio_meios = 0;     // meio segundo que sobrou da última atualização

//número do cartão que chega do sistema do professor ('S' 'C'), 6 dígitos + '\0'
static char cartao_inserido[7] = "";

//espera um byte da serial por até SERIAL_ESPERA_MS; retorna 1 se chegou
static unsigned char Serial_Esperar_Byte(unsigned char *dado) {
    for (unsigned char t = 0; t < SERIAL_ESPERA_MS; t++) {
        if (serial_disponivel()) {
            *dado = serial_ler();
            return 1;
        }
        Delay1ms();
    }
    return 0;
}

/* trata as mensagens que o sistema do professor manda sozinho (protocolo do Parkmicros)
   e responde cada uma, senão o aplicativo acusa falha:
     'S' 'H' dia mes ano(2 bytes) hora min  -> acerta data e hora, responde 'P' 'H'
     'S' 'V' n + n bytes (via e placas)     -> só lê, responde 'P' 'V'
     'S' 'C' "123456"                       -> guarda o cartão, responde 'P' 'C'
   retorna 1 quando a data e hora mudaram (pra atualizar o LCD) */
static unsigned char Servidor_Processar(void) {
    unsigned char tipo, n, dado;
    unsigned char dados[6];

    if (!serial_disponivel()) {
        return 0;
    }
    if (serial_ler() != 'S') {
        return 0;
    }
    if (!Serial_Esperar_Byte(&tipo)) {
        return 0;
    }

    switch (tipo) {
        case 'H':
            for (n = 0; n < 6; n++) {
                if (!Serial_Esperar_Byte(&dados[n])) {
                    return 0;   // mensagem pela metade, ignora
                }
            }
            dia    = dados[0];
            mes    = dados[1];
            ano    = ((unsigned int)dados[2] << 8) | dados[3];
            if (ano < 2000 || ano > 2099) {
                ano = ((unsigned int)dados[3] << 8) | dados[2];
            }
            hora   = dados[4];
            minuto = dados[5];
            segundo = 0;                    // a mensagem não manda segundos
            relogio_meios = 0;
            Relogio_Meios_Segundos();       // descarta o tempo contado antes de acertar o relógio
            hora_recebida = 1;
            serial_enviar('P');
            serial_enviar('H');
            return 1;

        case 'V':
            if (!Serial_Esperar_Byte(&n)) {
                return 0;
            }
            while (n--) {
                if (!Serial_Esperar_Byte(&dado)) {
                    return 0;
                }
            }
            serial_enviar('P');
            serial_enviar('V');
            return 0;

        case 'C':
            for (n = 0; n < 6; n++) {
                if (!Serial_Esperar_Byte(&dado)) {
                    return 0;
                }
                cartao_inserido[n] = dado;
            }
            cartao_inserido[6] = '\0';
            serial_enviar('P');
            serial_enviar('C');
            return 0;

        default:
            return 0;
    }
}

/* faz o relógio andar com o tempo contado pelo Timer1 (a cada 0,5 s, no LED_Alerta.c)
   o sistema do professor só acerta a hora; depois disso quem conta é o Arduino
   retorna 1 quando o minuto mudou (pra atualizar o LCD) */
static unsigned char Relogio_Atualizar(void) {
    unsigned int meios = Relogio_Meios_Segundos() + relogio_meios;
    unsigned char mudou = 0;

    if (!hora_recebida) {
        return 0;           // ainda não sabe a hora, não tem o que contar
    }

    relogio_meios = meios % 2;
    for (meios /= 2; meios > 0; meios--) {      // um segundo por vez
        if (++segundo < 60) continue;
        segundo = 0;
        mudou = 1;
        if (++minuto < 60) continue;
        minuto = 0;
        if (++hora < 24) continue;
        hora = 0;
        if (++dia <= Dias_Do_Mes(mes, ano)) continue;
        dia = 1;
        if (++mes <= 12) continue;
        mes = 1;
        ano++;
    }
    return mudou;
}

static void lcd_dois_digitos(unsigned char n) {
    lcd_putc('0' + n / 10);
    lcd_putc('0' + n % 10);
}

//tela de inatividade: data na linha de cima e hora na de baixo
static void Mostrar_Data_Hora(void) {
    lcd_clear();
    lcd_set_cursor(0, 0);
    if (!hora_recebida) {
        lcd_print("Aguardando hora");
        return;
    }
    lcd_dois_digitos(dia);
    lcd_putc('/');
    lcd_dois_digitos(mes);
    lcd_putc('/');
    lcd_dois_digitos(ano / 100);
    lcd_dois_digitos(ano % 100);

    lcd_set_cursor(0, 1);
    lcd_dois_digitos(hora);
    lcd_putc(':');
    lcd_dois_digitos(minuto);
}

int main (void){
unsigned char tecla, estado, tempo, data, pagamento;
unsigned char Pss_preferencial = 0;
char placa[8] = "";     // 7 letras/números + '\0'
//definir as milhoes de variaveis
lcd_init();
Teclado_Iniciar();
serial_init();

Timer0_Iniciar();
LED_Alerta_Iniciar();

tecla = SEM_TECLA;
while(1){
    Relogio_Atualizar();
    Mostrar_Data_Hora();
    tecla = Teclado_Ler();
    while (tecla != '1'){       // só a tecla 1 começa o cadastro do carro, as outras são ignoradas
        //função de mostar o horario no lcd
        //a hora é acertada pelo sistema do professor ('S' 'H') e o Arduino conta sozinho depois
        unsigned char atualizar = Servidor_Processar();
        if (Relogio_Atualizar()) {
            atualizar = 1;      // virou o minuto
        }
        if (atualizar) {
            Mostrar_Data_Hora();
        }
        tecla = Teclado_Ler();  // lê de novo, senão fica preso aqui pra sempre
    }

    //lcd "Digite a placa:", escrita com o multitap do teclado (* confirma, # apaga)
    //repete até a placa ter um formato válido (ou ser o código do administrador)
    while (1) {
        lcd_clear();
        lcd_set_cursor(0, 0);
        lcd_print("Digite a placa:");
        Teclado_Ler_Placa(placa, 0, 1);

        if (strcmp(placa, "ADMPARK") == 0 || Placa_Formato_Valido(placa)) {
            break;
        }
        lcd_clear();
        lcd_set_cursor(0, 0);
        lcd_print("Placa invalida!");
        DelayNms(2000);
    }

   if (strcmp(placa, "ADMPARK") == 0){     // string em C não compara com ==
        //pedir senha e confirmar
        //implementar as milhoes de funcionalidades ...
        continue;               // por enquanto volta pra tela de inatividade (administrador não paga)
   }

    Pss_preferencial = Placa_Preferencial(placa);   // 1 se idosa ou pcd

   //lcd "Quanto tempo:"
   // "0) 30 mins   1) 1h   2) 1h30    3)2h"
   //usei os valores baseados no valor de byte enviado para o professor 

    tecla = Teclado_Ler();
    tempo = tecla - '0';    // o teclado devolve o caractere '0'..'9', o switch usa o número 0..9

    //Calendário
    //Eu olhei tua função calendario, GIGANTESCA, mas minha ideia final ela virar uma flag 
    /*  1 - Dia de semana 
        2 - sabado 
        default - domingos/feriados
    */
    Relogio_Atualizar();    // a data tem que estar em dia antes de ver o tipo de dia
    data = Calendario_Flag(dia, mes, ano); //nome só pra referencia

    switch (data){
        case 1: 
            //dia de semana 
            switch (tempo){
                case 0: 
                    if (Pss_preferencial == 1){
                        // lcd "Valor: R$"
                        //define byte de valor para sistema do sor 
                    }else {
                        // lcd "Valor: R$"
                        //define byte de valor para sistema do sor 
                     
                    }
                    break; 
                case 1: 
                    if (Pss_preferencial == 1){
                        // lcd "Valor: R$"
                        //define byte de valor para sistema do sor 
                    }else {
                        // lcd "Valor: R$"
                        //define byte de valor para sistema do sor 
                    }
                    break;
                case 2: 
                    if (Pss_preferencial == 1){
                        // lcd "Valor: R$"
                        //define byte de valor para sistema do sor 
                    }else {
                        // lcd "Valor: R$"
                        //define byte de valor para sistema do sor 
                    }
                    break;
                case 3:
                    if (Pss_preferencial == 1){
                        // lcd "Valor: R$"
                        //define byte de valor para sistema do sor 
                    }else {
                        // lcd "Valor: R$"
                        //define byte de valor para sistema do sor 
                    }
                    break;
                default:
                //lcd "Esse entrada é inválida! "
                // fechar o sistema ???
                    break;
                }
            break;
        
        case 2 :
            //sabado 
            switch (tempo){
                case 0: 
                    if (Pss_preferencial == 1){
                        // lcd "Valor: R$"
                        //define byte de valor para sistema do sor 
                    }else {
                        // lcd "Valor: R$"
                        //define byte de valor para sistema do sor 
                    }
                    break; 
                case 1: 
                    if (Pss_preferencial == 1){
                        // lcd "Valor: R$"
                        //define byte de valor para sistema do sor 
                    }else {
                        // lcd "Valor: R$"
                        //define byte de valor para sistema do sor 
                    }
                    break;
                case 2: 
                    if (Pss_preferencial == 1){
                        // lcd "Valor: R$"
                        //define byte de valor para sistema do sor 
                    }else {
                        // lcd "Valor: R$"
                        //define byte de valor para sistema do sor 
                    }
                    break;
                case 3:
                    if (Pss_preferencial == 1){
                        // lcd "Valor: R$"
                        //define byte de valor para sistema do sor 
                    }else {
                        // lcd "Valor: R$"
                        //define byte de valor para sistema do sor 
                    }
                    break;
                default:
                //lcd "Esse estrada é inválida! "
                // fechar o sistema ???
                    break;
                }
            break; 
       
        default: 
            //domingos e feriados 
            // lcd "Valor: R$"
            //define byte de valor para sistema do sor
            break;
    } //não sei outra forma de fazer isso


    //Lcd "Forma de pagamento:"
    // "1) Dinheiro     2)Cartão    3)Cartão Azul"
    pagamento = Teclado_Ler() - '0';

    switch(pagamento){
        case 1: 
            //pagamento em dinheiro 
            //lcd "Insira o dinheiro:"
            //ler do sensor
            break; 
        case 2: 
            //pagamento em cartão, sistema do sor 
            //lcd "Numéro do cartão:"
            //lcd "Senha: ", os numeros não aparecem na tela
            //Enviar dados para o sor
            break; 
        case 3:
            //pagamento em cartão azul 
            //verifica se a placa está vinculada
            //fazer desconto do valor 
            break; 
        default:
            //lcd "Essa estrada não é válida."
            //fechar o sistema??
            break;
    }

    //lcd "Pagamento finalizado!"
    //inicio da contagem do tempo, não sei como fazer isso ainda 
    //tolerância de 3 mins
    //Led pisca 
    
}
}
