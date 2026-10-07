"""Teste dos feriados moveis do calendario.c contra uma tabela de referencia.

Verifica Carnaval, Quarta-feira de Cinzas, Sexta-feira Santa e Corpus Christi de 2000 a 2078.
A Pascoa nao eh feriado e o calendario.c nao devolve a data dela, entao ela eh verificada
indiretamente: todos os feriados acima sao calculados a partir dela.

O script compila o calendario.c como biblioteca compartilhada (precisa de um compilador C,
`cc` por padrao ou o da variavel de ambiente CC) e chama as funcoes C via ctypes.

Uso, a partir da raiz do projeto:
    python3 testes/teste_calendario.py
"""

import ctypes
import os
import subprocess
import sys
import tempfile
from datetime import date, timedelta
from pathlib import Path

RAIZ = Path(__file__).resolve().parent.parent
CALENDARIO_C = RAIZ / "calendario.c"

# tabela de referencia: ano -> datas no formato dia/mes
TABELA = {
    2000: {"pascoa": "23/04", "carnaval": "07/03", "corpus_christi": "22/06"},
    2001: {"pascoa": "15/04", "carnaval": "27/02", "corpus_christi": "14/06"},
    2002: {"pascoa": "31/03", "carnaval": "12/02", "corpus_christi": "30/05"},
    2003: {"pascoa": "20/04", "carnaval": "04/03", "corpus_christi": "19/06"},
    2004: {"pascoa": "11/04", "carnaval": "24/02", "corpus_christi": "10/06"},
    2005: {"pascoa": "27/03", "carnaval": "08/02", "corpus_christi": "26/05"},
    2006: {"pascoa": "16/04", "carnaval": "28/02", "corpus_christi": "15/06"},
    2007: {"pascoa": "08/04", "carnaval": "20/02", "corpus_christi": "07/06"},
    2008: {"pascoa": "23/03", "carnaval": "05/02", "corpus_christi": "22/05"},
    2009: {"pascoa": "12/04", "carnaval": "24/02", "corpus_christi": "11/06"},
    2010: {"pascoa": "04/04", "carnaval": "16/02", "corpus_christi": "03/06"},
    2011: {"pascoa": "24/04", "carnaval": "08/03", "corpus_christi": "23/06"},
    2012: {"pascoa": "08/04", "carnaval": "21/02", "corpus_christi": "07/06"},
    2013: {"pascoa": "31/03", "carnaval": "12/02", "corpus_christi": "30/05"},
    2014: {"pascoa": "20/04", "carnaval": "04/03", "corpus_christi": "19/06"},
    2015: {"pascoa": "05/04", "carnaval": "17/02", "corpus_christi": "04/06"},
    2016: {"pascoa": "27/03", "carnaval": "09/02", "corpus_christi": "26/05"},
    2017: {"pascoa": "16/04", "carnaval": "28/02", "corpus_christi": "15/06"},
    2018: {"pascoa": "01/04", "carnaval": "13/02", "corpus_christi": "31/05"},
    2019: {"pascoa": "21/04", "carnaval": "05/03", "corpus_christi": "20/06"},
    2020: {"pascoa": "12/04", "carnaval": "25/02", "corpus_christi": "11/06"},
    2021: {"pascoa": "04/04", "carnaval": "16/02", "corpus_christi": "03/06"},
    2022: {"pascoa": "17/04", "carnaval": "01/03", "corpus_christi": "16/06"},
    2023: {"pascoa": "09/04", "carnaval": "21/02", "corpus_christi": "08/06"},
    2024: {"pascoa": "31/03", "carnaval": "13/02", "corpus_christi": "30/05"},
    2025: {"pascoa": "20/04", "carnaval": "04/03", "corpus_christi": "19/06"},
    2026: {"pascoa": "05/04", "carnaval": "17/02", "corpus_christi": "04/06"},
    2027: {"pascoa": "28/03", "carnaval": "09/02", "corpus_christi": "27/05"},
    2028: {"pascoa": "16/04", "carnaval": "29/02", "corpus_christi": "15/06"},
    2029: {"pascoa": "01/04", "carnaval": "13/02", "corpus_christi": "31/05"},
    2030: {"pascoa": "21/04", "carnaval": "05/03", "corpus_christi": "20/06"},
    2031: {"pascoa": "13/04", "carnaval": "25/02", "corpus_christi": "12/06"},
    2032: {"pascoa": "28/03", "carnaval": "10/02", "corpus_christi": "27/05"},
    2033: {"pascoa": "17/04", "carnaval": "01/03", "corpus_christi": "16/06"},
    2034: {"pascoa": "09/04", "carnaval": "21/02", "corpus_christi": "08/06"},
    2035: {"pascoa": "25/03", "carnaval": "06/02", "corpus_christi": "24/05"},
    2036: {"pascoa": "13/04", "carnaval": "26/02", "corpus_christi": "12/06"},
    2037: {"pascoa": "05/04", "carnaval": "17/02", "corpus_christi": "04/06"},
    2038: {"pascoa": "25/04", "carnaval": "09/03", "corpus_christi": "24/06"},
    2039: {"pascoa": "10/04", "carnaval": "22/02", "corpus_christi": "09/06"},
    2040: {"pascoa": "01/04", "carnaval": "14/02", "corpus_christi": "31/05"},
    2041: {"pascoa": "21/04", "carnaval": "05/03", "corpus_christi": "20/06"},
    2042: {"pascoa": "06/04", "carnaval": "18/02", "corpus_christi": "05/06"},
    2043: {"pascoa": "29/03", "carnaval": "10/02", "corpus_christi": "28/05"},
    2044: {"pascoa": "17/04", "carnaval": "01/03", "corpus_christi": "16/06"},
    2045: {"pascoa": "09/04", "carnaval": "21/02", "corpus_christi": "08/06"},
    2046: {"pascoa": "25/03", "carnaval": "06/02", "corpus_christi": "24/05"},
    2047: {"pascoa": "14/04", "carnaval": "26/02", "corpus_christi": "13/06"},
    2048: {"pascoa": "05/04", "carnaval": "18/02", "corpus_christi": "04/06"},
    2049: {"pascoa": "18/04", "carnaval": "02/03", "corpus_christi": "17/06"},
    2050: {"pascoa": "10/04", "carnaval": "22/02", "corpus_christi": "09/06"},
    2051: {"pascoa": "02/04", "carnaval": "14/02", "corpus_christi": "01/06"},
    2052: {"pascoa": "21/04", "carnaval": "05/03", "corpus_christi": "20/06"},
    2053: {"pascoa": "06/04", "carnaval": "18/02", "corpus_christi": "05/06"},
    2054: {"pascoa": "29/03", "carnaval": "10/02", "corpus_christi": "28/05"},
    2055: {"pascoa": "18/04", "carnaval": "02/03", "corpus_christi": "17/06"},
    2056: {"pascoa": "02/04", "carnaval": "15/02", "corpus_christi": "01/06"},
    2057: {"pascoa": "22/04", "carnaval": "06/03", "corpus_christi": "21/06"},
    2058: {"pascoa": "14/04", "carnaval": "26/02", "corpus_christi": "13/06"},
    2059: {"pascoa": "30/03", "carnaval": "11/02", "corpus_christi": "29/05"},
    2060: {"pascoa": "18/04", "carnaval": "02/03", "corpus_christi": "17/06"},
    2061: {"pascoa": "10/04", "carnaval": "22/02", "corpus_christi": "09/06"},
    2062: {"pascoa": "26/03", "carnaval": "07/02", "corpus_christi": "25/05"},
    2063: {"pascoa": "15/04", "carnaval": "27/02", "corpus_christi": "14/06"},
    2064: {"pascoa": "06/04", "carnaval": "19/02", "corpus_christi": "05/06"},
    2065: {"pascoa": "29/03", "carnaval": "10/02", "corpus_christi": "28/05"},
    2066: {"pascoa": "11/04", "carnaval": "23/02", "corpus_christi": "10/06"},
    2067: {"pascoa": "03/04", "carnaval": "15/02", "corpus_christi": "02/06"},
    2068: {"pascoa": "22/04", "carnaval": "06/03", "corpus_christi": "21/06"},
    2069: {"pascoa": "14/04", "carnaval": "26/02", "corpus_christi": "13/06"},
    2070: {"pascoa": "30/03", "carnaval": "11/02", "corpus_christi": "29/05"},
    2071: {"pascoa": "19/04", "carnaval": "03/03", "corpus_christi": "18/06"},
    2072: {"pascoa": "10/04", "carnaval": "23/02", "corpus_christi": "09/06"},
    2073: {"pascoa": "26/03", "carnaval": "07/02", "corpus_christi": "25/05"},
    2074: {"pascoa": "15/04", "carnaval": "27/02", "corpus_christi": "14/06"},
    2075: {"pascoa": "07/04", "carnaval": "19/02", "corpus_christi": "06/06"},
    2076: {"pascoa": "19/04", "carnaval": "03/03", "corpus_christi": "18/06"},
    2077: {"pascoa": "11/04", "carnaval": "23/02", "corpus_christi": "10/06"},
    2078: {"pascoa": "03/04", "carnaval": "15/02", "corpus_christi": "02/06"},
}

# inclui o calendario.c inteiro para ter acesso as funcoes static (ex.: pascoa());
# o main dele eh renomeado para nao conflitar, e teste_pascoa() eh exportada para o Python
WRAPPER_C = """
#define main calendario_main
#include "{caminho}"
#undef main

bool teste_pascoa(uint8_t dia, uint8_t mes, uint16_t ano) {{
  return pascoa(dia, mes, ano, bissexto(ano));
}}
"""


def ler_data(texto, ano):
    dia, mes = texto.split("/")
    return date(ano, int(mes), int(dia))


def compilar_calendario(pasta):
    wrapper = Path(pasta) / "wrapper.c"
    biblioteca = Path(pasta) / "calendario.so"
    wrapper.write_text(WRAPPER_C.format(caminho=CALENDARIO_C.as_posix()))

    compilador = os.environ.get("CC", "cc")
    subprocess.run([compilador, "-Wall", "-Wextra", "-shared", "-fPIC",
                    "-o", str(biblioteca), str(wrapper)], check=True)

    lib = ctypes.CDLL(str(biblioteca))
    lib.teste_pascoa.argtypes = [ctypes.c_uint8, ctypes.c_uint8, ctypes.c_uint16]
    lib.teste_pascoa.restype = ctypes.c_bool
    return lib


def e_feriado_movel(lib, data):
    return lib.teste_pascoa(data.day, data.month, data.year)


def formatar_falha(ano, nome, data, motivo):
    return f"FALHOU  {ano}  {nome:<24} {data:%d/%m}  {motivo}"


def verificar_ano(lib, ano, datas):
    """Verifica um ano da tabela. Retorna (quantidade de verificacoes, lista de falhas)."""
    pascoa = ler_data(datas["pascoa"], ano)
    carnaval = ler_data(datas["carnaval"], ano)
    corpus = ler_data(datas["corpus_christi"], ano)
    verificacoes = 0
    falhas = []

    # 1) sanidade da propria tabela: Carnaval = Pascoa - 47 e Corpus Christi = Pascoa + 60
    verificacoes += 2
    if pascoa - timedelta(days=47) != carnaval:
        falhas.append(formatar_falha(ano, "tabela: Carnaval", carnaval,
                                     "nao fica 47 dias antes da Pascoa da tabela"))
    if pascoa + timedelta(days=60) != corpus:
        falhas.append(formatar_falha(ano, "tabela: Corpus Christi", corpus,
                                     "nao fica 60 dias depois da Pascoa da tabela"))

    # 2) cada feriado movel esperado tem que ser reconhecido por pascoa()
    esperados = {
        carnaval: "Carnaval",
        carnaval + timedelta(days=1): "Quarta-feira de Cinzas",
        pascoa - timedelta(days=2): "Sexta-feira Santa",
        corpus: "Corpus Christi",
    }
    for data, nome in esperados.items():
        verificacoes += 1
        if not e_feriado_movel(lib, data):
            falhas.append(formatar_falha(ano, nome, data,
                                         "esperado feriado, pascoa() retornou falso"))

    # 3) nenhum outro dia do ano pode ser marcado como feriado movel
    data = date(ano, 1, 1)
    while data.year == ano:
        if data not in esperados:
            verificacoes += 1
            if e_feriado_movel(lib, data):
                falhas.append(formatar_falha(ano, "feriado movel a mais", data,
                                             "pascoa() retornou verdadeiro num dia que nao eh feriado"))
        data += timedelta(days=1)

    return verificacoes, falhas


def main():
    total_verificacoes = 0
    total_falhas = 0

    with tempfile.TemporaryDirectory() as pasta:
        lib = compilar_calendario(pasta)

        for ano, datas in TABELA.items():
            verificacoes, falhas = verificar_ano(lib, ano, datas)
            total_verificacoes += verificacoes
            total_falhas += len(falhas)
            for falha in falhas:
                print(falha)

    print(f"\n{total_verificacoes} verificacoes, {total_falhas} falha(s), "
          f"anos {min(TABELA)} a {max(TABELA)}")
    return 0 if total_falhas == 0 else 1


if __name__ == "__main__":
    sys.exit(main())
