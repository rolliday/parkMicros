//pra tentar organizar minhas ideias 
//preferi colocar aq só pra não sair desarrumando tudo na main 

#define TAM_SENHA 5 

static void ADMPARK () {
  char senha [TAM_SENHA + 1]; 
  unsigned char adm; 

  lcd_clear();
  lcd_set_cursor(0, 0);
  lcd_print("Administrador");
  lcd_set_cursor(0, 1); 
  lcd_print("Senha: ");
  Campo_Ler(senha, 5, 1, 7, 1);

  if (strcmp(senha, "12345") == 0 ){
    lcd_clear();
    lcd_set_cursor(0, 0);
    lcd_print("0:D/H 1:Inad");
    lcd_set_cursor(0, 1);
    lcd_print("2:Fer 3:Bloq"); //essas coisa me parecem grande demais... 
    
    do {
        tecla = Esperar_Tecla();
        adm = tecla - '0';    
    } while (tecla < '0' || tecla > '3'); 

    /* assumo que não sei como fazer essas funções, eu pensei em fazer uma lista para feriados adicionados aq e datas de bloquei,
    mas não sei como integrar isso ao código de calendario... pedi pra ia ajudar sksksk
    data e hora eu tb não sei como modificar 
    basicamente não sei nadaaa skssks
      */
    switch (adm){
      case 0: 
        //modificar data ou hora 
        break; 
      case 1: 
        lcd_clear();
        lcd_set_cursor(0, 0);
        lcd_print("Ocorrencia :");
        lcd_set_cursor(0, 1);
        lcd_print(dados_ocorrencia);
        //peguei da ia ....
        //cuidar que eu mexi no teu servidor processar, se der errado é uma modificação no caso V 
        break; 
      case 2: { // Adicionar feriados especiais 
        char buf_fer[5];
        lcd_clear();
        lcd_set_cursor(0, 0);
        lcd_print("Digite Feriado");
        lcd_set_cursor(0, 1);
        lcd_print("DDMM: ");
        Campo_Ler(buf_fer, 4, 0, 6, 1);
        
        if (strlen(buf_fer) == 4) {
            uint8_t d = (buf_fer[0] - '0') * 10 + (buf_fer[1] - '0');
            uint8_t m = (buf_fer[2] - '0') * 10 + (buf_fer[3] - '0');
            
            if (m >= 1 && m <= 12 && d >= 1 && d <= Dias_Do_Mes(m, ano)) {
                if (adicionar_feriado_extra(d, m)) {
                    lcd_clear();
                    lcd_set_cursor(0, 0);
                    lcd_print("Feriado salvo!");
                } else {
                    lcd_clear();
                    lcd_set_cursor(0, 0);
                    lcd_print("Lista cheia!");
                }
            } else {
                lcd_clear();
                lcd_set_cursor(0, 0);
                lcd_print("Data invalida!");
            }
            Esperar_Ms(1500);
        }
        break;
      }  
      case 3: { // Bloquear datas 
        char buf_bloq[9];
        lcd_clear();
        lcd_set_cursor(0, 0);
        lcd_print("Data Bloqueio");
        lcd_set_cursor(0, 1);
        lcd_print("DDMMAAAA:");
        Campo_Ler(buf_bloq, 8, 0, 0, 1);
        
        if (strlen(buf_bloq) == 8) {
            uint8_t d  = (buf_bloq[0] - '0') * 10 + (buf_bloq[1] - '0');
            uint8_t m  = (buf_bloq[2] - '0') * 10 + (buf_bloq[3] - '0');
            uint16_t a = (buf_bloq[4] - '0') * 1000 + (buf_bloq[5] - '0') * 100 + (buf_bloq[6] - '0') * 10 + (buf_bloq[7] - '0');
            
            if (a >= 2000 && m >= 1 && m <= 12 && d >= 1 && d <= Dias_Do_Mes(m, a)) {
                if (adicionar_data_bloqueada(d, m, a)) {
                    lcd_clear();
                    lcd_set_cursor(0, 0);
                    lcd_print("Data bloqueada!");
                } else {
                    lcd_clear();
                    lcd_set_cursor(0, 0);
                    lcd_print("Lista cheia!");
                }
            } else {
                lcd_clear();
                lcd_set_cursor(0, 0);
                lcd_print("Data invalida!");
            }
            Esperar_Ms(1500);
        }
        break;
      }
    }
  }else {
    lcd_clear();
    lcd_set_cursor(0, 0);
    lcd_print("Senha incorreta!");
    Esperar_Ms (2000); 
    break; 
  }
} 
