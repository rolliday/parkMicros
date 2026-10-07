#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>

// no Arduino usa o PROGMEM de verdade; no PC (para testar) finge que ele nao existe
#ifdef __AVR__
  #include <avr/pgmspace.h>
#else
  #define PROGMEM
  #define pgm_read_byte(addr) (*(addr))
#endif

// dias de cada mes (ano nao bissexto), guardado so na flash
const uint8_t meses[12] PROGMEM = {31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};

typedef struct {
  uint8_t dia;
  uint8_t mes;
} Feriado_dia_mes;

// calcula se o ano é bissexto (true) ou não (false)
// so eh valido para anos entre 1901 e 2099
bool bissexto(uint16_t a) {
  return (a % 4) == 0;
}

// calcula quantos anos bissextos existem entre 2000 (inclusive) e o ano fornecido (exclusive)
// so eh valido para anos entre 2000 e 2099
static uint8_t quantos_bissextos(uint16_t a) {
  return (a - 2000 + 3) / 4;
}

// calcula qual dia da semana eh a partir do 01/01/2000 (sabado)
uint8_t dia_da_semana(uint8_t dia_final, uint8_t mes_final, uint16_t ano_final, bool flag_bissexto) {
  const uint16_t ano_inicial = 2000;
  // uint16_t: em 2099 o total chega a ~36500 dias, o que estoura um int de 16 bits (AVR)
  uint16_t a = 0;

  // calcula quantos dias tiveram no ano até o mes fornecido
  for (uint8_t i = 0; i < mes_final - 1; i++) {
    a += pgm_read_byte(&meses[i]);
  }

  // calcula quandos dias tiveram até o ano fornecido
  a += (uint16_t)(ano_final - ano_inicial) * 365u + quantos_bissextos(ano_final);

  // calcula quantos dias tiveram até o dia fornecido
  if (flag_bissexto && mes_final > 2) {
    a += dia_final;
  } else {
    a += dia_final - 1;
  }

  // calcula o dia da semana
  // 0 = sabado, 1 = domingo, 2 = segunda-feira, 3 = terca-feira, 4 = quarta-feira, 5 = quinta-feira, 6 = sexta-feira
  return a % 7;
}

// calcula qual dia do ano eh (sem considerar o 29/02; a correcao do bissexto eh feita por quem chama)
static uint16_t dia_do_ano(uint8_t dia, uint8_t mes) {
  uint16_t a = 0;

  // calcula quantos dias tiveram no ano até o mes fornecido
  for (uint8_t i = 0; i < mes - 1; i++) {
    a += pgm_read_byte(&meses[i]);
  }

  // calcula quantos dias tiveram até o dia fornecido
  a += dia;

  return a;
}

static Feriado_dia_mes calculo_carnaval(uint16_t dia_do_ano_pascoa, bool flag_bissexto) {
  Feriado_dia_mes res;
  res.mes = 0;
  // pascoa cai entre 22/03 (dia 81) e 25/04 (dia 115), entao o resultado fica entre 34 e 68
  res.dia = dia_do_ano_pascoa - 47;

  if (res.dia <= 59) {
    res.mes = 2;
    res.dia -= 31;
  } else if (res.dia <= 90) {
    res.mes = 3;
    res.dia -= 59;
  }

  if (flag_bissexto && res.mes == 2) {
    res.dia++;
  }

  return res;
}

static Feriado_dia_mes calculo_corpus_christi(uint8_t dia_pascoa, uint8_t mes_pascoa) {
  Feriado_dia_mes res;

  if (dia_pascoa != 1) {
    res.mes = mes_pascoa + 2;
    res.dia = dia_pascoa - 1;
  } else {
    res.mes = mes_pascoa + 1;
    res.dia = dia_pascoa + 30;
  }

  return res;
}

// verifica se hoje eh carnaval, quarta-feira de cinzas, sexta-feira santa ou nao 
// so eh valido para anos entre 1900 e 2099 (Formula de Gauss)
static bool pascoa(uint8_t dia, uint8_t mes, uint16_t ano, bool flag_bissexto) {
  uint8_t dia_pascoa, mes_pascoa;
  const uint8_t x = 24, y = 5;
  uint8_t a = ano % 19, b = ano % 4, c = ano % 7;
  uint8_t d = (19 * a + x) % 30;
  uint8_t e = (2 * b + 4 * c + 6 * d + y) % 7;

  if (d + e > 9) {
    dia_pascoa = d + e - 9;
    mes_pascoa = 4;
  } else {
    dia_pascoa = d + e + 22;
    mes_pascoa = 3;
  }

  // excecoes para anos especiais onde a pascoa cai em datas diferentes do calculo de Gauss
  if (ano == 2049) {
    dia_pascoa = 18;
  }
  if (ano == 2076) {
    dia_pascoa = 19;
  }
  
  // calculo para saber quando cai a terca-feira de carnaval
  Feriado_dia_mes carnaval = calculo_carnaval(dia_do_ano(dia_pascoa, mes_pascoa), flag_bissexto);

  // calculo para saber quando cai a quarta-feira de cinzas
  uint8_t quarta_cinzas = carnaval.dia + 1;

  // calculo para saber quando cai a sexta-feira santa
  int8_t sexta_santa = dia_pascoa - 2;

  // calculo paara saber quando cai o Corpus Christi
  Feriado_dia_mes corpus = calculo_corpus_christi(dia_pascoa, mes_pascoa);

  if (dia == carnaval.dia && mes == carnaval.mes) {          // Carnaval
    return 1;
  } else if (dia == quarta_cinzas && mes == carnaval.mes) {  // Quarta-feira de Cinzas
    return 1;
  } else if (dia == sexta_santa && mes == mes_pascoa) {      // Sexta-feira Santa
    return 1;
  } else if (dia == corpus.dia && mes == corpus.mes) {       // Corpus Christi
    return 1;
  } else {                                                   // Nenhum feriado
    return 0;
  }
}

// verifica se hoje eh feriado ou nao
bool feriado(uint8_t dia, uint8_t mes, uint16_t ano, bool flag_bissexto) {
  if (pascoa(dia, mes, ano, flag_bissexto)) { // Chamada para calcular os feriados dependentes da pascoa
    return 1;
  } else if (dia == 1 && mes == 1) {          // Ano Novo
    return 1; 
  } else if (dia == 21 && mes == 4) {         // Tiradentes
    return 1; 
  } else if (dia == 1 && mes == 5) {          // Dia do Trabalho
    return 1;  
  } else if (dia == 7 && mes == 9) {          // Independência do Brasil
    return 1; 
  } else if (dia == 12 && mes == 10) {        // Nossa Senhora Aparecida
    return 1; 
  } else if (dia == 2 && mes == 11) {         // Finados
    return 1; 
  } else if (dia == 15 && mes == 11) {        // Proclamação da República
    return 1; 
  } else if (dia == 20 && mes == 11) {        // Dia da Consciência Negra
    return 1; 
  } else if (dia == 25 && mes == 12) {        // Natal
    return 1; 
  } else {
    return 0;
  }
}

int main() {
  uint8_t dia = 3, mes = 3;
  uint16_t ano = 2076;
  bool flag_bissexto = bissexto(ano);

  bool saida = feriado(dia, mes, ano, flag_bissexto);
  printf("%d\n", saida);

  uint8_t dia_semana = dia_da_semana(dia, mes, ano, flag_bissexto);

  switch (dia_semana) {
    case 0:
      printf("Sabado\n");
      break;
    case 1:
      printf("Domingo\n");
      break;
    case 2:
      printf("Segunda-feira\n");
      break;
    case 3:
      printf("Terca-feira\n");
      break;
    case 4:
      printf("Quarta-feira\n");
      break;
    case 5:
      printf("Quinta-feira\n");
      break;
    case 6:
      printf("Sexta-feira\n");
      break;
  }
  return 0;
}

// adicionar testes