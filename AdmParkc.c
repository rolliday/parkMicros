//pra tentar organizar minhas ideias 
//preferi colocar aq só pra não sair desarrumando tudo na main 

#define TAM_SENHA 5 

static void ADMPARK () {
  char senha [TAM_SENHA + 1]; 
  unsigned char adm; 
  unsigned char tecla; 

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
        // puramente ia, nem sei o que ta acontecendo sssjs
        char buf_dh[13];
        lcd_clear();
        lcd_set_cursor(0, 0);
        lcd_print("Nova Data/Hora");
        lcd_set_cursor(0, 1);
        lcd_print("DDMMAAAAHHMM:");
        
        // Vamos ler 12 caracteres (0 = sem ocultar senha/texto)
        Campo_Ler(buf_dh, 12, 0, 0, 1); 
        
        if (strlen(buf_dh) == 12) {
            uint8_t d  = (buf_dh[0] - '0') * 10 + (buf_dh[1] - '0');
            uint8_t m  = (buf_dh[2] - '0') * 10 + (buf_dh[3] - '0');
            uint16_t a = (buf_dh[4] - '0') * 1000 + (buf_dh[5] - '0') * 100 + (buf_dh[6] - '0') * 10 + (buf_dh[7] - '0');
            uint8_t h  = (buf_dh[8] - '0') * 10 + (buf_dh[9] - '0');
            uint8_t mn = (buf_dh[10] - '0') * 10 + (buf_dh[11] - '0');
            
            // Valida se a hora, minuto e a data existem de verdade
            if (h < 24 && mn < 60 && a >= 2000 && m >= 1 && m <= 12 && d >= 1 && d <= Dias_Do_Mes(m, a)) {
                dia = d;
                mes = m;
                ano = a;
                hora = h;
                minuto = mn;
                segundo = 0;
                relogio_meios = 0; // Reseta a contagem de meio segundo do timer
                
                lcd_clear();
                lcd_set_cursor(0, 0);
                lcd_print("Hora Atualizada!");
            } else {
                lcd_clear();
                lcd_set_cursor(0, 0);
                lcd_print("Valores invalidos!");
            }
            Esperar_Ms(1500);
        }
        break; 
      case 1: 
        lcd_clear();
        lcd_set_cursor(0, 0);
        lcd_print("Ocorrencia :");
        lcd_set_cursor(0, 1);
        lcd_print(dados_ocorrencia);
        //saida de ia, tentei adaptar ....
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
    return; 
  }
} 
