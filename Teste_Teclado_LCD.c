//para testar o teclado e o lcd, a vendo a senha como *****
#include <avr/io.h>
#include <util/delay.h>
#include <stdint.h>

//pinagem 
  #define COLUNA_DDR     DDRB
  #define COLUNA_PORT    PORTB
  #define COLUNA_PIN     PINB
  #define COLUNA_BASE    PB2
  #define LINHA_DDR      DDRC
  #define LINHA_PORT     PORTC
  #define LINHA_BASE     PC0

  #define LCD_CTRL_DDR   DDRB
  #define LCD_CTRL_PORT  PORTB
  #define LCD_RS         PB0
  #define LCD_E          PB1
  #define LCD_DATA_DDR   DDRD
  #define LCD_DATA_PORT  PORTD      

#define COLUNA_MASCARA ((1 << COLUNA_BASE) | (1 << (COLUNA_BASE + 1)) | (1 << (COLUNA_BASE + 2)))
#define LINHA_MASCARA  ((1 << LINHA_BASE)  | (1 << (LINHA_BASE + 1))  | (1 << (LINHA_BASE + 2)) | (1 << (LINHA_BASE + 3)))

#define F_CPU 16000000UL
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

//busquei ideias, mas eles usavam uma biblioteca do arduino e eu não sei se pode 
//pedi ajuda pra ia, acho mais tranquilo nessa parte pois é só função de inicialização 
static void lcd_pulse_enable(void) {
    LCD_CTRL_PORT |= (1 << LCD_E);
    __asm__ __volatile__("nop\n nop\n nop\n nop\n nop\n nop\n nop\n nop\n");  /* 500 ns */
    LCD_CTRL_PORT &= ~(1 << LCD_E);
}

static void lcd_write_nibble(unsigned char nibble) {
    LCD_DATA_PORT = (LCD_DATA_PORT & 0x0F) | ((nibble & 0x0F) << 4);
    lcd_pulse_enable();
}

static void lcd_send(unsigned char valor, unsigned char rs) {
    if (rs) LCD_CTRL_PORT |=  (1 << LCD_RS);
    else    LCD_CTRL_PORT &= ~(1 << LCD_RS);

    lcd_write_nibble(valor >> 4);
    lcd_write_nibble(valor);
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
    LCD_CTRL_DDR |= (1 << LCD_RS) | (1 << LCD_E);
    LCD_DATA_DDR |= 0xF0;
    LCD_CTRL_PORT &= ~((1 << LCD_RS) | (1 << LCD_E));

    DelayNms(50);                   /* > 40 ms após Vcc estável */

    lcd_write_nibble(0x03);
    DelayNms(5);                    /* > 4,1 ms */
    lcd_write_nibble(0x03);
    Delay40us_x(3);                 /* 120 us (> 100 us) */
    lcd_write_nibble(0x03);
    Delay40us();
    lcd_write_nibble(0x02);         /* passa para 4 bits */
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