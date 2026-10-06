#include <stdio.h>

typedef struct {
  int dia;
  int mes;
} Carnaval;

// calcula se o ano é bissexto (1) ou não (0)
int bissexto(int a) {
  int b = a % 4;
  if (b == 0) {
    return 1;
  } else {
    return 0;
  }
}

// calcula quantos anos bissextos existem entre 2000 e o ano fornecido usando a função bissexto(int a)
int quantos_bissextos(int a) {
  int b = 0;

  for (int i = 2000; i < a; i++) {
    if (bissexto(i)) {
      b++;
    }
  }

  return b;
}

// calcula qual dia da semana eh a partir do 01/01/2000 (sabado)
int dia_da_semana(int dia_final, int mes_final, int ano_final, int flag_bissexto) {
  int dia_inicial = 1, mes_inicial = 1, ano_inicial = 2000;
  int meses[] = {31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};
  int a = 0;

  // calcula quantos dias tiveram no ano até o mes fornecido
  for (int i = 0; i < mes_final - 1; i++) {
    a += meses[i];
  }

  // calcula quandos dias tiveram até o ano fornecido
  a += (ano_final - ano_inicial) * 365 + quantos_bissextos(ano_final);

  // calcula quantos dias tiveram até o dia fornecido
  if (flag_bissexto && mes_final > 2) {
    a += dia_final;
  } else {
    a += dia_final - 1;
  }

  // calcula o dia da semana
  // 0 = sabado, 1 = domingo, 2 = segunda-feira, 3 = terca-feira, 4 = quarta-feira, 5 = quinta-feira, 6 = sexta-feira
  a = a % 7;

  return a;
}

// calcula qual dia do ano eh
int dia_do_ano(int dia, int mes, int flag_bissexto) {
  int meses[] = {31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};
  int a = 0;

  // calcula quantos dias tiveram no ano até o mes fornecido
  for (int i = 0; i < mes - 1; i++) {
    a += meses[i];
  }

  // calcula quantos dias tiveram até o dia fornecido
  if (flag_bissexto && mes > 2) {
    a += dia;
  } else {
    a += dia;
  }

  return a;
}

Carnaval calculo_carnaval(int dia_pascoa, int mes_pascoa, int dia_do_ano, int ano, int flag_bissexto) {
  Carnaval res;
  res.dia = 0, res.mes = 0;
  res.dia = dia_do_ano - 47;

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

// verifica se hoje eh pascoa ou nao (so eh valido para anos entre 1900 e 2099) - Formula de Gauss
int pascoa(int dia, int mes, int ano, int flag_bissexto) {
  int dia_pascoa = 0, mes_pascoa = 0;
  int x = 24, y = 5;
  int a = ano % 19, b = ano % 4, c = ano % 7;
  int d = (19 * a + x) % 30;
  int e = (2 * b + 4 * c + 6 * d + y) % 7;

  if (d + e > 9) {
    dia_pascoa = d + e - 9;
    mes_pascoa = 4;
  } else {
    dia_pascoa = d + e + 22;
    mes_pascoa = 3;
  }
  
  Carnaval r = calculo_carnaval(dia_pascoa, mes_pascoa, dia_do_ano(dia_pascoa, mes_pascoa, ano), ano, flag_bissexto);
  printf("Carnaval: %d/%d\n", r.dia, r.mes);
  return 0;
}

// verifica se hoje eh feriado ou nao
int feriado(int dia, int mes) {


  if (dia == 1 && mes == 1) {          // Ano Novo
    return 1; 
  } else if (dia == 21 && mes == 4) {  // Tiradentes
    return 1; 
  } else if (dia == 1 && mes == 5) {   // Dia do Trabalho
    return 1;  
  } else if (dia == 7 && mes == 9) {   // Independência do Brasil
    return 1; 
  } else if (dia == 12 && mes == 10) { // Nossa Senhora Aparecida
    return 1; 
  } else if (dia == 2 && mes == 11) {  // Finados
    return 1; 
  } else if (dia == 15 && mes == 11) { // Proclamação da República
    return 1; 
  } else if (dia == 20 && mes == 11) { // Dia da Consciência Negra
    return 1; 
  } else if (dia == 25 && mes == 12) { // Natal
    return 1; 
  } else {
    return 0;
  }
}

int main() {
  int dia = 5, mes = 10, ano = 2026;
  int flag_bissexto = 0;

  flag_bissexto = bissexto(ano);

  int saida = pascoa(dia, mes, ano, flag_bissexto);
  printf("%d\n", saida);
  return 0;
}

// falta adicionar o calculo de Corpus Christi e as 2 exceções
// adicionar testes
// trocar int por ponteiros