"""Teste da validacao do formato da placa.

Verifica a Placa_Formato_Valido() do Comparacao_Placas.c: os formatos Denatran (ABC1234)
e Mercosul (BRA0S17) tem que ser aceitos, e o resto (so numero, so letra, faltando ou
sobrando posicao...) recusado. A digitacao da placa no teclado (multitap) eh testada na placa.

O script compila o Comparacao_Placas.c como biblioteca compartilhada (precisa de um compilador C,
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
DADOS_C = RAIZ / "Dados_Placas_Cartoes.c"   # tem as listas que o Comparacao_Placas.c usa

# placa -> True se o formato tem que ser aceito
TABELA = {
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
    "": False,          # * sem nada digitado
    "1BC1234": False,
    "ABCD234": False,
    "ABC1S2A": False,
    "abc1234": False,   # o teclado so gera maiuscula
    "AB 1234": False,
}


def compilar_placas(pasta):
    biblioteca = Path(pasta) / "placas.so"

    compilador = os.environ.get("CC", "cc")
    subprocess.run([compilador, "-Wall", "-Wextra", "-shared", "-fPIC",
                    "-o", str(biblioteca), str(COMPARACAO_C), str(DADOS_C)], check=True)

    lib = ctypes.CDLL(str(biblioteca))
    lib.Placa_Formato_Valido.argtypes = [ctypes.c_char_p]
    lib.Placa_Formato_Valido.restype = ctypes.c_byte
    return lib


def main():
    total_falhas = 0

    # no Windows a biblioteca continua carregada e nao da pra apagar a pasta no fim;
    # ignore_cleanup_errors deixa a pasta temporaria para tras em vez de quebrar o teste
    with tempfile.TemporaryDirectory(ignore_cleanup_errors=True) as pasta:
        lib = compilar_placas(pasta)

        for placa, esperado in TABELA.items():
            obtido = bool(lib.Placa_Formato_Valido(placa.encode()))
            if obtido != esperado:
                total_falhas += 1
                print(f"FALHOU  {placa!r:<11} esperado {'valida' if esperado else 'invalida'}, "
                      f"deu {'valida' if obtido else 'invalida'}")

    print(f"\n{len(TABELA)} verificacoes, {total_falhas} falha(s)")
    return 0 if total_falhas == 0 else 1


if __name__ == "__main__":
    sys.exit(main())
