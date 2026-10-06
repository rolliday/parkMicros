# Projeto 1 — ParkMicros

## Periféricos Utilizados
- Teclado 3x4
- LCD 16x2
- Sensor para pagamento em dinheiro
- LED

Comunicação serial com o aplicativo do professor.

## Tabela de Valores a Pagar

| Opção | 0 — 30 min | 1 — 1h | 2 — 1h30 | 3 — 2h | 4 — ilimitado |
|---|---|---|---|---|---|
| Seg-sex 8h-18h | 2,50 | 3,50 | 4,50 | 6,00 | — |
| Idoso/PCD | — | 0 | 2,00 | 3,50 | 4,50 |
| Sábados 8h-12h | 1,50 | 2,50 | 4,00 | 4,00 | — |
| Dom/feriados | — | 0 | 0 | 0 | 0 |

> Nota: a tabela original do PDF não deixa claro a divisão exata de todas as colunas (algumas linhas trazem um valor a mais que outras). Os valores acima foram organizados com base na extração do texto original e na definição de modalidades (0 a 4) encontrada na seção de protocolo serial — vale conferir contra o PDF original antes de usar.

**Tempo limite:** 2h, exceto para idoso e PCD, com 3 minutos de tolerância.

## Teclado Alfanumérico
- Entrada de placas (padrão antigo de telefone)
- Escolha de horário/forma de pagamento
- Digitação de cartão (6 dígitos) e senha (5 dígitos, exibidos como `*****`)

### Formato de Placas
- Denatran: `ABC1234`
- Mercosul: `BRA0S17`

## Display LCD
- Sem interação: mostra o horário
- Mostra quando informações são inválidas
- Nunca mostra senhas

## Ruas Permitidas para Estacionamento
- Sarmento Leite
- Osvaldo Aranha
- João Pessoa

> Observação do autor: o sistema do professor envia a informação de onde o carro foi estacionado, mas isso não fazia sentido no contexto do projeto.

## Formas de Pagamento
- **Dinheiro:** via sensor (tensão em cima do sensor)
- **Cartão:** informação enviada por comunicação serial para o sistema do professor
- **Cartão azul:** vinculado à placa, debitado diretamente no sistema interno; se não houver saldo, avisar o usuário

## Funções do Administrador
Código de acesso: `ADMPARK - 123445`

- Troca de horário do sistema
- Acesso à placa e rua do carro inadimplente
- Registro de feriados especiais e datas de bloqueio

## Passo a Passo do Sistema
1. Mostrar o horário quando inativo
2. Clicar em qualquer tecla para sair da inatividade
3. Digitar a placa
4. Conferir a placa e verificar se é pessoa idosa ou PCD
5. Tempo de escolha (horário)
6. Forma de pagamento:
   - Dinheiro: tensão em cima do sensor
   - Cartão: sistema do professor
   - Cartão azul: sistema interno
7. "Impressão de nota" — confirmação

### Tolerância e Atraso
- Tolerância de 3 minutos.
- Caso extrapole, um LED pisca na frequência de 1 Hz.
- Se pago em cartão azul, é debitado o valor de mais 30 minutos a cada atraso.

## Cálculo de Data
O sistema deve calcular o dia da semana com base na informação de que **01/01/2000 foi um sábado**.

### Feriados Nacionais
- 01/01
- Paixão de Cristo (data móvel)
- 21/04
- 01/05
- 07/09
- 12/10
- 02/11
- 15/11
- 20/11
- 25/12

## Protocolo de Comunicação Serial

### Envio de dados de estacionamento (nós → ele)
`'P' 'E' "ABC1234" "0"` — 10 bytes

**Modalidade:**
| Código | Significado |
|---|---|
| 0 | 30 min |
| 1 | 1h |
| 2 | 1h30 |
| 3 | 2h |
| 4 | Ilimitado |

Resposta (ele → nós): `'S' 'E'` — 2 bytes

### Envio de requisição de pagamento (nós → ele)
`'P' 'P' "cartão" "senha" "valor"` — 16 bytes

Valor em centavos, como centena (ex.: 2,50 → `250`).

Resposta (ele → nós): `'S' 'P' ?` — 3 bytes

| Resposta | Significado |
|---|---|
| `SPS` | Sucesso |
| `SPF` | Falha (dados incorretos) |
| `SPI` | Falha (saldo insuficiente) |

### Envio de dados de impressão (nós → ele)
`'P' 'I' n ....` — n + 4 bytes

Resposta (ele → nós): `'S' 'I'` — 2 bytes

### Envio de hora (ele → nós)
`'S' 'H' dia mes ano hora minuto` — 8 bytes

Resposta (nós → ele): `'P' 'H'` — 2 bytes

### Envio de condição de via (ele → nós)
`'S' 'V' n "via" "placa"` — n + 3 bytes

Resposta (nós → ele): `'P' 'V'` — 2 bytes
