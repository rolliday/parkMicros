"""Teste da digitacao e da validacao da placa.

Verifica duas coisas:
  1) Placa_Formato_Valido() do Comparacao_Placas.c: formatos Denatran (ABC1234) e
     Mercosul (BRA0S17) aceitos, o resto recusado.
  2) Teclado_Ler_Placa() do Teclado 4x3.c: o multitap (2 -> A, B, C, 2...), a confirmacao
     da letra por 1 s parado ou por outra tecla, o # apagando, o * terminando e a 8a tecla
     sendo ignorada. Tambem confere o que aparece no LCD.

O Teclado 4x3.c eh compilado inteiro, sem mudanca, com um avr/io.h falso: o PINB responde
como se a tecla do roteiro estivesse apertada na linha que o codigo ativou, e o tempo eh
simulado (cada Delay1ms anda 1 ms). Assim o teste passa pela varredura do teclado, pelo
debounce e pelo multitap de verdade. Fios e LCD reais continuam sendo testados na placa.

O script compila os .c como bibliotecas compartilhadas (precisa de um compilador C,
`cc` por padrao ou o da variavel de ambiente CC) e chama as funcoes C via ctypes.

Uso, a partir da raiz do projeto:
    python3 testes/teste_placa.py
"""

import ctypes
import os
import subprocess
import sys
import tempfile
from pathlib import Path

RAIZ = Path(__file__).resolve().parent.parent
COMPARACAO_C = RAIZ / "Comparacao_Placas.c"
DADOS_C = RAIZ / "Dados_Placas_Cartoes.c"
TECLADO_C = RAIZ / "Teclado 4x3.c"

# placa -> True se o formato tem que ser aceito
FORMATOS = {
    "ABC1234": True,    # Denatran
    "BRA0S17": True,    # Mercosul
    "IOS0098": True,
    "PEI1Z92": True,
    "ADMPARK": False,   # codigo do administrador (a main trata antes de validar)
    "1234567": False,   # so numero
    "AAAAAAA": False,   # so letra
    "ABC123": False,    # faltando posicao
    "ABC12345": False,  # posicao a mais
    "AB": False,
    "": False,
    "1BC1234": False,
    "ABCD234": False,
    "ABC1S2A": False,
    "abc1234": False,   # o teclado so gera maiuscula
    "AB 1234": False,
}

# roteiros do teclado: cada caractere eh um aperto de tecla, "." eh uma pausa de 1,5 s
# (confirma a letra pelo tempo) e espaco nao faz nada -> (placa esperada, descricao)
ROTEIROS = {
    "2.22.222 1 2222 3333 4444 *": ("ABC1234", "Denatran, letras da mesma tecla separadas por pausa"),
    "22 777 99# 2 0 7777 1 77777 *": ("BRA0S17", "Mercosul, # apagando a letra que ainda nao confirmou"),
    "6.# 99.999.9999 1 2222 3333 4444 5 *": ("XYZ1234", "# apagando letra ja confirmada, 8a tecla ignorada"),
    "2.2.2.1 2222 3333 4444 *": ("AAA1234", "letra confirmada por 1 s parado"),
    "2 33 *": ("AE", "* com menos de 7 termina na hora (a main recusa)"),
    "2.2.*": ("AA", "* depois das letras confirmarem pelo tempo"),
    "*": ("", "* sem nada digitado"),
    "0 1 #*": ("0", "# apagando o ultimo, 0 e 1 sao so numero"),
}

APERTO_MS = 80      # quanto tempo a tecla fica apertada
INTERVALO_MS = 200  # entre um aperto e outro (bem menos que 1 s, nao confirma a letra)
PAUSA_MS = 1500     # o "." do roteiro (mais que 1 s, confirma a letra)

# o PINB, o TCNT0 e o TIFR0 viram funcoes do teste; o resto vira variavel comum
FAKE_IO_H = """
#include <stdint.h>
extern volatile uint8_t DDRB, PORTB, DDRD, PORTD, TCCR0A, TCCR0B;
uint8_t fake_pinb(void);
volatile uint8_t *fake_tcnt0(void);
volatile uint8_t *fake_tifr0(void);
#define PINB  fake_pinb()
#define TCNT0 (*fake_tcnt0())
#define TIFR0 (*fake_tifr0())
#define PB0 0
#define PB1 1
#define PB2 2
#define PD4 4
#define PD5 5
#define PD6 6
#define PD7 7
#define CS00 0
#define CS01 1
#define TOV0 0
"""

FAKE_DELAY_H = """
void fake_delay_us(double us);
#define _delay_us(us) fake_delay_us(us)
#define _delay_ms(ms) fake_delay_us((ms) * 1000.0)
"""

# inclui o Teclado 4x3.c inteiro e simula o hardware em volta dele;
# teste_ler_placa() eh exportada para o Python
WRAPPER_C = """
#include <setjmp.h>
#include <string.h>
#include "{caminho}"

#define MAX_TECLAS 64

volatile uint8_t DDRB, PORTB, DDRD, PORTD, TCCR0A, TCCR0B;
static volatile uint8_t tcnt0, tifr0;

static unsigned long agora_us;      // relogio simulado
static unsigned long limite_us;     // passou disso e a funcao nao terminou: o teste interrompe ela
static jmp_buf interromper;
static char tecla_r[MAX_TECLAS];
static unsigned long inicio_r[MAX_TECLAS], fim_r[MAX_TECLAS];
static int n_r;

static char tela[17];               // linha do LCD onde a placa aparece
static unsigned char cursor_col;

void lcd_set_cursor(unsigned char col, unsigned char linha) {{
  (void)linha;
  cursor_col = col;
}}

void lcd_putc(char c) {{
  if (cursor_col < 16) tela[cursor_col] = c;
  cursor_col++;
}}

// anda o relogio; se a funcao passou do limite (travada esperando algo), sai dela na marra
static void andar(unsigned long us) {{
  agora_us += us;
  if (agora_us > limite_us) longjmp(interromper, 1);
}}

static char tecla_apertada(void) {{
  for (int i = 0; i < n_r; i++) {{
    if (agora_us >= inicio_r[i] && agora_us < fim_r[i]) return tecla_r[i];
  }}
  return 0;
}}

// mesma ordem do mapa_teclas: linha = posicao / 3, coluna = posicao % 3
uint8_t fake_pinb(void) {{
  static const char mapa[] = "123456789*0#";
  uint8_t pinb = 0xFF;
  char t = tecla_apertada();
  andar(5);         // cada leitura leva um tempinho, senao o "espera soltar" nunca acaba
  if (t) {{
    int pos = (int)(strchr(mapa, t) - mapa);
    if ((PORTD & (1 << (4 + pos / 3))) == 0) {{   // a linha da tecla esta ativa (em 0)
      pinb &= (uint8_t)~(1 << (pos % 3));
    }}
  }}
  return pinb;
}}

volatile uint8_t *fake_tcnt0(void) {{    // o Delay1ms recarrega o TCNT0 uma vez: anda 1 ms
  andar(1000);
  return &tcnt0;
}}

volatile uint8_t *fake_tifr0(void) {{    // o estouro do Timer0 ja aconteceu
  tifr0 = 1 << TOV0;
  return &tifr0;
}}

void fake_delay_us(double us) {{
  andar((unsigned long)us);
}}

// retorna 1 se a funcao nao terminou (ficou presa) ate 5 s depois da ultima tecla
int teste_ler_placa(const char *teclas, const int *inicio_ms, const int *duracao_ms, int n,
                    char *placa, char *lcd) {{
  n_r = n;
  for (int i = 0; i < n; i++) {{
    tecla_r[i] = teclas[i];
    inicio_r[i] = (unsigned long)inicio_ms[i] * 1000;
    fim_r[i] = inicio_r[i] + (unsigned long)duracao_ms[i] * 1000;
  }}
  limite_us = (n ? fim_r[n - 1] : 0) + 5000000UL;
  agora_us = 0;
  PORTD = 0xF0;
  memset(tela, ' ', 16);
  tela[16] = '\\0';

  if (setjmp(interromper)) {{       // chegou aqui pelo longjmp: a funcao travou
    memcpy(lcd, tela, 17);
    return 1;
  }}
  Teclado_Ler_Placa(placa, 0, 1);

  memcpy(lcd, tela, 17);
  return 0;
}}
"""


def compilar(pasta, nome, fontes, extras=()):
    biblioteca = Path(pasta) / f"{nome}.so"
    compilador = os.environ.get("CC", "cc")
    subprocess.run([compilador, "-Wall", "-Wextra", "-shared", "-fPIC", "-funsigned-char",
                    *extras, "-o", str(biblioteca), *map(str, fontes)], check=True)
    return ctypes.CDLL(str(biblioteca))


def compilar_placas(pasta):
    lib = compilar(pasta, "placas", [COMPARACAO_C, DADOS_C])
    lib.Placa_Formato_Valido.argtypes = [ctypes.c_char_p]
    lib.Placa_Formato_Valido.restype = ctypes.c_byte
    return lib


def compilar_teclado(pasta):
    (Path(pasta) / "avr").mkdir()
    (Path(pasta) / "util").mkdir()
    (Path(pasta) / "avr" / "io.h").write_text(FAKE_IO_H)
    (Path(pasta) / "util" / "delay.h").write_text(FAKE_DELAY_H)
    wrapper = Path(pasta) / "wrapper_teclado.c"
    wrapper.write_text(WRAPPER_C.format(caminho=TECLADO_C.as_posix()))

    lib = compilar(pasta, "teclado", [wrapper], ["-I", str(pasta)])
    lib.teste_ler_placa.argtypes = [ctypes.c_char_p, ctypes.POINTER(ctypes.c_int),
                                    ctypes.POINTER(ctypes.c_int), ctypes.c_int,
                                    ctypes.c_char_p, ctypes.c_char_p]
    lib.teste_ler_placa.restype = ctypes.c_int
    return lib


def montar_roteiro(texto):
    """Transforma "2.22 *" em listas de (tecla, inicio em ms, duracao em ms)."""
    teclas, inicios, duracoes = [], [], []
    tempo = INTERVALO_MS
    for c in texto:
        if c == " ":
            continue
        if c == ".":
            tempo += PAUSA_MS
            continue
        teclas.append(c)
        inicios.append(tempo)
        duracoes.append(APERTO_MS)
        tempo += APERTO_MS + INTERVALO_MS
    return teclas, inicios, duracoes


def ler_placa(lib, roteiro):
    teclas, inicios, duracoes = montar_roteiro(roteiro)
    n = len(teclas)
    placa = ctypes.create_string_buffer(8)
    lcd = ctypes.create_string_buffer(17)
    travou = lib.teste_ler_placa("".join(teclas).encode(), (ctypes.c_int * n)(*inicios),
                                 (ctypes.c_int * n)(*duracoes), n, placa, lcd)
    return placa.value.decode(), lcd.value.decode(), travou


def verificar_formatos(lib):
    falhas = []
    for placa, esperado in FORMATOS.items():
        obtido = bool(lib.Placa_Formato_Valido(placa.encode()))
        if obtido != esperado:
            falhas.append(f"FALHOU  formato  {placa!r:<11} esperado "
                          f"{'valida' if esperado else 'invalida'}, deu {'valida' if obtido else 'invalida'}")
    return len(FORMATOS), falhas


def verificar_teclado(lib):
    falhas = []
    for roteiro, (esperado, descricao) in ROTEIROS.items():
        placa, lcd, travou = ler_placa(lib, roteiro)
        if travou:
            falhas.append(f"FALHOU  teclado  {descricao}: a funcao ficou presa e nao terminou com o * do roteiro")
            continue
        if placa != esperado:
            falhas.append(f"FALHOU  teclado  {descricao}: placa {placa!r}, esperado {esperado!r}")
        elif lcd[:8] != esperado.ljust(8):
            falhas.append(f"FALHOU  teclado  {descricao}: LCD {lcd[:8]!r}, esperado {esperado.ljust(8)!r}")
    return len(ROTEIROS), falhas


def main():
    total_verificacoes = 0
    total_falhas = 0

    # no Windows a biblioteca continua carregada e nao da pra apagar a pasta no fim;
    # ignore_cleanup_errors deixa a pasta temporaria para tras em vez de quebrar o teste
    with tempfile.TemporaryDirectory(ignore_cleanup_errors=True) as pasta:
        for verificar, compilar_lib in ((verificar_formatos, compilar_placas),
                                        (verificar_teclado, compilar_teclado)):
            verificacoes, falhas = verificar(compilar_lib(pasta))
            total_verificacoes += verificacoes
            total_falhas += len(falhas)
            for falha in falhas:
                print(falha)

    print(f"\n{total_verificacoes} verificacoes, {total_falhas} falha(s) "
          f"({len(FORMATOS)} formatos de placa, {len(ROTEIROS)} roteiros de teclado)")
    return 0 if total_falhas == 0 else 1


if __name__ == "__main__":
    sys.exit(main())
