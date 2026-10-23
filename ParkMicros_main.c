//minha versão da main
#ifndef F_CPU
#define F_CPU 16000000UL    // tem que vir antes do <util/delay.h>, senão o _delay_ms conta errado
#endif

#include <avr/io.h>
#include <util/delay.h>
#include <string.h>

#define SEM_TECLA   0x00    // tem que ser o mesmo valor do Teclado 4x3.c

//funções dos outros arquivos (todos os .c ficam como Compile no projeto)
void Timer0_Iniciar(void);                  // Teclado 4x3.c
void Teclado_Iniciar(void);                 // Teclado 4x3.c
unsigned char Teclado_Ler(void);            // Teclado 4x3.c
void lcd_init(void);                        // Teste_Teclado_LCD.c
void LED_Alerta_Iniciar(void);              // LED_Alerta.c (Timer1)
unsigned char Calendario_Flag(unsigned char dia, unsigned char mes, unsigned int ano);   // calendario.c

//a main não roda quando algum teste está ligado (símbolo TESTE_... definido no projeto)
#if !defined(TESTE_TECLADO) && !defined(TESTE_TECLADO_LCD) && !defined(TESTE_BOTOES) && \
    !defined(TESTE_PLACAS) && !defined(TESTE_DADOS) && !defined(TESTE_CALENDARIO)
int main (void){
unsigned char tecla, estado, tempo, data, pagamento;
unsigned char Pss_preferencial = 0;
char placa[8] = "";     // 7 letras/números + '\0'
unsigned char dia = 1, mes = 1;     // TODO: a data vem do sistema do professor pela serial ('S' 'H')
unsigned int ano = 2026;
//definir as milhoes de variaveis
lcd_init();
Teclado_Iniciar();

Timer0_Iniciar();
LED_Alerta_Iniciar();

tecla = SEM_TECLA;
while(1){
    tecla = Teclado_Ler();
    while (tecla == SEM_TECLA){
        //função de mostar o horario no lcd
        //a gente pega de comunicação serial com o computador do professor??
        tecla = Teclado_Ler();  // lê de novo, senão fica preso aqui pra sempre
    }

    /*lcd "Digite a placa:"
    escrever a placa, Multitap do teclado 
    verificar se o padrão de placa esta correto e se a pss é idosa/pcd 
    Pss_preferencial = 1; se idosa ou pcd 
   */

   if (strcmp(placa, "ADMPARK") == 0){     // string em C não compara com ==
        //pedir senha e confirmar 
        //implementar as milhoes de funcionalidades ...
   }

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
#endif
