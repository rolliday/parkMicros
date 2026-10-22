//para testar o teclado e o lcd, a vendo a senha como *****
#ifndef F_CPU
#define F_CPU 16000000UL    // tem que vir antes do <util/delay.h>, senão o _delay_us conta errado
#endif

#include <avr/io.h>
#include <util/delay.h>
#include <stdint.h>

/* pinagem
    teclado: pinos 1 a 7 do teclado nos pinos 4 a 10 do Arduino (igual ao Teclado 4x3.c)
      linhas  -> PD4..PD7 (pinos 4 a 7)
      colunas -> PB0..PB2 (pinos 8 a 10)
    LCD com módulo I2C (PCF8574):
      SDA -> A4 (PC4)
      SCL -> A5 (PC5)
*/
  #define COLUNA_DDR     DDRB
  #define COLUNA_PORT    PORTB
  #define COLUNA_PIN     PINB
  #define COLUNA_BASE    PB0
  #define LINHA_DDR      DDRD
  #define LINHA_PORT     PORTD
  #define LINHA_BASE     PD4

//endereço do módulo I2C: depende do chip, o lcd_init testa os dois
  #define LCD_I2C_END_1  0x27       // PCF8574T, o mais comum
  #define LCD_I2C_END_2  0x3F       // PCF8574AT
  #define TWI_TWBR_100K  72         // TWBR = (F_CPU / 100 kHz - 16) / 2

//o que cada saída do PCF8574 liga no LCD (P4..P7 vão no D4..D7)
  #define LCD_RS         0x01       // P0
  #define LCD_E          0x04       // P2
  #define LCD_LUZ        0x08       // P3, luz de fundo

#define COLUNA_MASCARA ((1 << COLUNA_BASE) | (1 << (COLUNA_BASE + 1)) | (1 << (COLUNA_BASE + 2)))
#define LINHA_MASCARA  ((1 << LINHA_BASE)  | (1 << (LINHA_BASE + 1))  | (1 << (LINHA_BASE + 2)) | (1 << (LINHA_BASE + 3)))

#define BOUNCE          10
#define T0_RELOAD_1MS   6
#define T0_PRESCALE_64  ((1 << CS01) | (1 << CS00))
#define T2_OCR_40US     79
#define T2_PRESCALE_8   (1 << CS21)

#define NUM_LINHAS      4
#define NUM_COLUNAS     3
#define SEM_TECLA       0x00
#define INVALIDO        2          
#define TECLA_FIM       '*'  

#define CARTAO_DIGITOS  6
#define SENHA_DIGITOS   5
#define CARTAO_COL      8
#define SENHA_COL       7

//comandos lcd
#define LCD_CLEAR        0x01
#define LCD_HOME         0x02
#define LCD_ENTRY_MODE   0x06
#define LCD_DISPLAY_OFF  0x08
#define LCD_DISPLAY_ON   0x0C
#define LCD_FUNC_4BIT_2L 0x28
#define LCD_SET_DDRAM    0x80

static unsigned char mapa_teclas[NUM_LINHAS][NUM_COLUNAS] =
{
    { '1', '2', '3' },
    { '4', '5', '6' },
    { '7', '8', '9' },
    { '*', '0', '#' }
};

void Timer0_Iniciar(void) {
    TCCR0A = 0x00;
    TCCR0B = T0_PRESCALE_64;
}

void Timer2_Iniciar(void) {
    TCCR2A = (1 << WGM21);          
    TCCR2B = T2_PRESCALE_8;
    OCR2A  = T2_OCR_40US;
}

void Delay1ms(void) {
    TCNT0 = T0_RELOAD_1MS;
    TIFR0 = (1 << TOV0);
    while ((TIFR0 & (1 << TOV0)) == 0);
    TIFR0 = (1 << TOV0);
}

void DelayNms(unsigned int n) {
    while (n--) {
        Delay1ms();
    }
}

void Delay40us(void) {
    TCNT2 = 0;
    TIFR2 = (1 << OCF2A);
    while ((TIFR2 & (1 << OCF2A)) == 0);
}

void Delay40us_x(unsigned char n) {
    while (n--) {
        Delay40us();
    }
}

//usei o teclado que fiz antes 
unsigned char Debouncer(unsigned char pino) {
    unsigned char count = 0;
    unsigned char anterior = INVALIDO;
    unsigned char atual;

    while (1) {
        Delay1ms();
        atual = (COLUNA_PIN >> pino) & 0x01;
        if (atual == anterior) {
            count++;
        } else {
            count = 0;
        }
        if (count == BOUNCE) {
            return atual;
        }
        anterior = atual;
    }
}

void Teclado_Iniciar(void) {
    COLUNA_DDR  &= ~COLUNA_MASCARA;
    COLUNA_PORT |=  COLUNA_MASCARA;

    LINHA_DDR  |= LINHA_MASCARA;
    LINHA_PORT |= LINHA_MASCARA;

    Timer0_Iniciar();
}

unsigned char Teclado_Ler(void) {
    unsigned char linha, coluna, pino;
    unsigned char tecla = SEM_TECLA;

    for (linha = 0; linha < NUM_LINHAS; linha++) {
        LINHA_PORT &= ~(1 << (LINHA_BASE + linha));   
        _delay_us(5);                                 

        for (coluna = 0; coluna < NUM_COLUNAS; coluna++) {
            pino = COLUNA_BASE + coluna;

            if ((COLUNA_PIN & (1 << pino)) == 0) {
                if (Debouncer(pino) == 0) {
                    tecla = mapa_teclas[linha][coluna];
                    while ((COLUNA_PIN & (1 << pino)) == 0);  
                    DelayNms(BOUNCE);
                    break;
                }
            }
        }

        LINHA_PORT |= (1 << (LINHA_BASE + linha)); 
        if (tecla != SEM_TECLA) {
            break;
        }
    }
    return tecla;
}

//I2C (TWI) do ATmega, só escrita, que é o que o LCD precisa
void TWI_Iniciar(void) {
    TWSR = 0x00;                // prescaler 1
    TWBR = TWI_TWBR_100K;       // 100 kHz
    TWCR = (1 << TWEN);
}

static void twi_esperar(void) {
    while ((TWCR & (1 << TWINT)) == 0);
}

//manda o START e o endereço; retorna 1 se o módulo respondeu (ACK)
static unsigned char twi_start(unsigned char endereco) {
    TWCR = (1 << TWINT) | (1 << TWSTA) | (1 << TWEN);
    twi_esperar();
    TWDR = endereco << 1;       // bit 0 = 0 -> escrita
    TWCR = (1 << TWINT) | (1 << TWEN);
    twi_esperar();
    return (TWSR & 0xF8) == 0x18;   // 0x18 = endereço enviado e ACK recebido
}

static void twi_escrever(unsigned char dado) {
    TWDR = dado;
    TWCR = (1 << TWINT) | (1 << TWEN);
    twi_esperar();
}

static void twi_stop(void) {
    TWCR = (1 << TWINT) | (1 << TWSTO) | (1 << TWEN);
    while (TWCR & (1 << TWSTO));
}

//busquei ideias, mas eles usavam uma biblioteca do arduino e eu não sei se pode
//pedi ajuda pra ia, acho mais tranquilo nessa parte pois é só função de inicialização
static unsigned char lcd_endereco = LCD_I2C_END_1;

//o LCD lê o dado na descida do E, então manda o mesmo byte com E em 1 e depois com E em 0
//cada byte no I2C leva ~90 us, bem mais que os 450 ns que o E precisa ficar em 1
static void lcd_write_nibble(unsigned char nibble, unsigned char rs) {
    unsigned char dado = ((nibble & 0x0F) << 4) | LCD_LUZ | (rs ? LCD_RS : 0);

    twi_start(lcd_endereco);
    twi_escrever(dado | LCD_E);
    twi_escrever(dado);
    twi_stop();
}

static void lcd_send(unsigned char valor, unsigned char rs) {
    lcd_write_nibble(valor >> 4, rs);
    lcd_write_nibble(valor, rs);
    Delay40us();
}

static void lcd_cmd(unsigned char cmd){ 
    lcd_send(cmd, 0);
}

void lcd_putc(char c){
    lcd_send((unsigned char)c, 1);
}

void lcd_print(const char *s) {
    while (*s) lcd_putc(*s++);
}

void lcd_clear(void) {
    lcd_cmd(LCD_CLEAR);
    DelayNms(2);
}

void lcd_home(void) {
    lcd_cmd(LCD_HOME);
    DelayNms(2);
}

/* col: 0..15 | linha: 0..1 */
void lcd_set_cursor(unsigned char col, unsigned char linha) {
    lcd_cmd(LCD_SET_DDRAM | ((linha ? 0x40 : 0x00) + col));
}

void lcd_init(void) {
    TWI_Iniciar();
    DelayNms(50);                   /* > 40 ms após Vcc estável */

    //descobre o endereço do módulo: se o 0x27 não responder, usa o 0x3F
    if (twi_start(LCD_I2C_END_1)) {
        lcd_endereco = LCD_I2C_END_1;
    } else {
        lcd_endereco = LCD_I2C_END_2;
    }
    twi_stop();

    lcd_write_nibble(0x03, 0);
    DelayNms(5);                    /* > 4,1 ms */
    lcd_write_nibble(0x03, 0);
    Delay40us_x(3);                 /* 120 us (> 100 us) */
    lcd_write_nibble(0x03, 0);
    Delay40us();
    lcd_write_nibble(0x02, 0);      /* passa para 4 bits */
    Delay40us();

    lcd_cmd(LCD_FUNC_4BIT_2L);
    lcd_cmd(LCD_DISPLAY_OFF);
    lcd_cmd(LCD_CLEAR);
    DelayNms(2);
    lcd_cmd(LCD_ENTRY_MODE);
    lcd_cmd(LCD_DISPLAY_ON);
}

//teste para vizualização de cartão e senha 
//fiz um teste tendo o "*" como um ok, mas da pra fazer fixando todos os tamanhos 
//e tb tem a posssibilidade de "#" ser um backspace (acho valido, mas ter mais problemas não seria algo que quero)
static void Campo_Ler(char *buf, unsigned char tam, unsigned char oculto, unsigned char col, unsigned char linha) {
    unsigned char n = 0;
    unsigned char tecla;

    buf[0] = '\0';
    lcd_set_cursor(col, linha);

    while (1) {
        tecla = Teclado_Ler();

        if (tecla == SEM_TECLA) {
            continue;   //usado pra volar no inicio do while 
        }
        if (tecla == TECLA_FIM) {
            if (n == tam) 
            {                 
                break;
            }
            continue;   
        }
        if (tecla >= '0' && tecla <= '9' && n < tam) {
            buf[n++] = tecla;
            buf[n] = '\0';
            lcd_putc(oculto ? '*' : tecla); //senha "*****"
        }
    }
}

 //cartao -> 6 dígitos, buffer com 7 posições
 //senha  -> 5 dígitos, buffer com 6 posições 
void Leitura_Cartao_Senha(char *cartao, char *senha) {
    lcd_clear();
    lcd_set_cursor(0, 0);
    lcd_print("Cartao: ");
    lcd_set_cursor(0, 1);
    lcd_print("Senha: ");

    Campo_Ler(cartao, CARTAO_DIGITOS, 0, CARTAO_COL, 0);
    Campo_Ler(senha,  SENHA_DIGITOS,  1, SENHA_COL,  1);
}

int main(void) {
    char cartao[CARTAO_DIGITOS + 1];
    char senha[SENHA_DIGITOS + 1];

    Timer0_Iniciar();
    Timer2_Iniciar();
    lcd_init();
    Teclado_Iniciar();

    while (1) {
        Leitura_Cartao_Senha(cartao, senha);   // bloqueia até os dois campos

        lcd_clear();
        lcd_set_cursor(0, 0);
        lcd_print("Dados lidos!");
        DelayNms(2000);
    }
}