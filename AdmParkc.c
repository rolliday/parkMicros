//função AdmPark 

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
    lcd_print("2:Fer 3:Bloq"); 
    
    do {
        tecla = Esperar_Tecla();
        adm = tecla - '0';    
    } while (tecla < '0' || tecla > '3'); 

    switch (adm){
      case 0: { // trocar data e hora
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
      }
      case 1: { // inadimplentes: placa e rua de cada carro em atraso, 3 s cada
        unsigned char c, achou = 0;
        for (c = 0; c < MAX_CARROS; c++) {
            if (carros[c].placa[0] == '\0' || !carros[c].atrasado) {
                continue;
            }
            achou = 1;
            lcd_clear();
            lcd_set_cursor(0, 0);
            lcd_print("Inad: ");
            lcd_print(carros[c].placa);
            lcd_set_cursor(0, 1);
            lcd_print("Rua: ");
            lcd_print(carros[c].rua == RUA_DESCONHECIDA ? "?" : nome_rua[carros[c].rua]);
            Esperar_Ms(3000);
        }
        if (!achou) {
            lcd_clear();
            lcd_set_cursor(0, 0);
            lcd_print("Nenhuma");
            lcd_set_cursor(0, 1);
            lcd_print("ocorrencia");
            Esperar_Ms(2000);
        }
        break; 
      }
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
