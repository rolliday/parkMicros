//minha versão da main
#include <avr/io.h>
#include <util/delay.h>

#define F_CPU 16000000UL
#define BOUNCE 10  
#define T0_RELOAD_1MS  6
#define T0_PRESCALE_64 ((1 << CS01) | (1 << CS00))

#define NUM_LINHAS  4
#define NUM_COLUNAS 3
#define SEM_TECLA   0x00
#define INVALIDO    2 


int main (void){
unsigned char tecla, estado, tempo, Pss_preferencial, data, pagamento; 
//definir as milhoes de variaveis 
Iniciar_LCD
Teclado_Iniciar; 

Timer0_Iniciar;
Timer1_Iniciar; 

tecla = SEM_TECLA; 
while(1){
    tecla = Teclado_Ler; 
    while (tecla == SEM_TECLA){
        //função de mostar o horario no lcd
        //a gente pega de comunicação serial com o computador do professor??
    }

    /*lcd "Digite a placa:"
    escrever a placa, Multitap do teclado 
    verificar se o padrão de placa esta correto e se a pss é idosa/pcd 
    Pss_preferencial = 1; se idosa ou pcd 
   */

   if (placa == "ADMPARK"){
        //pedir senha e confirmar 
        //implementar as milhoes de funcionalidades ...
   }

   //lcd "Quanto tempo:"
   // "0) 30 mins   1) 1h   2) 1h30    3)2h"
   //usei os valores baseados no valor de byte enviado para o professor 

    tecla = Teclado_Ler; 
    tecla = tempo; 

    //Calendário
    //Eu olhei tua função calendario, GIGANTESCA, mas minha ideia final ela virar uma flag 
    /*  1 - Dia de semana 
        2 - sabado 
        default - domingos/feriados
    */
    data = Função_Calendário; //nome só pra referencia 

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
                }
            break; 
       
        default: 
            //domingos e feriados 
            // lcd "Valor: R$"
            //define byte de valor para sistema do sor 
    
    } //não sei outra forma de fazer isso 


    //Lcd "Forma de pagamento:"
    // "1) Dinheiro     2)Cartão    3)Cartão Azul"
    pagamento = Teclado_Ler; 

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
    }

    //lcd "Pagamento finalizado!"
    //inicio da contagem do tempo, não sei como fazer isso ainda 
    //tolerância de 3 mins
    //Led pisca 
    
}
}
