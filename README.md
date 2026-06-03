# LPII 2026.1 - Trabalho Prático 1

## Identificação
- Nome: Miguel de Queiroz Fernandes Soares
- Matrícula: 20240008109

## Problema escolhido
**P1 - Multiplicação de matrizes (n x n).**

O programa calcula `C = A x B` com a implementação sequencial e com pthreads.
Na versão paralela, cada thread recebe um bloco de linhas de `C`, sem escrita concorrente na mesma posição.
O código mede apenas a região de computação com `CLOCK_MONOTONIC`, descarta a primeira execução (aquecimento) e calcula média.
Ao final, compara resultado sequencial e paralelo com tolerância para ponto flutuante (`1e-6`) e imprime `OK` ou `FALHA`.

## Estrutura
```text
.
├── CMakeLists.txt
├── README.md
├── results/
│   └── speedup.png
├── scripts/
│   └── plot_speedup.py
└── src/
    └── main.c
```

## Compilação

### Opção 1 - CMake
```bash
cmake -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build
```

### Opção 2 - comando único gcc
```bash
gcc -O2 -Wall -Wextra -pthread src/*.c -o lp2_p1 && ./lp2_p1 4
```

## Execução
Formato:
```bash
./lp2_p1 <threads> [n] [runs]
```

Exemplos:
```bash
./lp2_p1 4
./lp2_p1 8 1200 6
```

- `threads`: número de threads da versão paralela (obrigatório)
- `n`: dimensão da matriz (`n x n`, padrão `1000`)
- `runs`: número total de execuções (padrão `6`, primeira descartada)

## Saída esperada
O programa imprime:
- `T_seq médio`
- `T_par médio`
- `Speedup (T_seq/T_par)`
- `Checksum` das duas versões
- validação automática (`OK`/`FALHA`)

## Ambiente de teste
- CPU: Intel Core i7-1255U (12th Gen) — arquitetura híbrida: 2 P-cores + 8 E-cores
- Núcleos físicos/lógicos: 10 / 12
- Sistema operacional: Windows 11 Pro 10.0.26200
- Compilador: GCC 15.2.0 (MSYS2/ucrt64)
- Flags: `-O2 -Wall -Wextra -pthread`
- Dimensão usada nas medições: n = 1000, 6 execuções (primeira descartada), média de 5

## Tabela de resultados (Q3)

T_seq = 2.352 s (média de 5 execuções, versão sequencial pura).
Speedup calculado pelo programa como T_seq / T_par medidos na mesma execução.

| Threads | T_par (s) | Speedup |
|---|---:|---:|
| 1 (seq) | 2.352 | 1.00x |
| 2 | 0.985 | 1.50x |
| 4 | 0.794 | 2.16x |
| 8 | 0.524 | 3.19x |

## Q4 - Estudo de escalabilidade (extra)

### Tabela completa

| Threads | T_par (s) | Speedup | Eficiência |
|---|---:|---:|---:|
| 1 | 2.293 | 1.03x | 103% |
| 2 | 0.985 | 1.50x | 75% |
| 4 | 0.794 | 2.16x | 54% |
| 8 | 0.524 | 3.19x | 40% |
| 10 | 0.581 | 3.16x | 32% |

> T_seq de referência: 2.352 s. Eficiência = Speedup / número de threads.

### Gráfico

![Speedup e Eficiência x Threads](results/speedup.png)

Para gerar o gráfico novamente:
```bash
python scripts/plot_speedup.py
```

### Discussão

O speedup cresce de forma sublinear e satura em torno de 3.2x, mesmo com até 10 threads
disponíveis. Três fatores explicam esse comportamento. Primeiro, a **Lei de Amdahl**: existe
uma parcela sequencial inevitável no programa (criação e junção das threads, validação dos
resultados, impressão), que impõe um teto ao speedup máximo independentemente do número de
threads. Segundo, o **overhead de criação e junção de threads**: a cada medição o programa
chama `pthread_create` e `pthread_join` T vezes; esse custo fixo por execução cresce com T e
reduz o ganho líquido, especialmente visível entre 8 e 10 threads. Terceiro, a
**arquitetura híbrida P/E-cores do i7-1255U**: o processador possui 2 P-cores rápidos
(com Hyper-Threading) e 8 E-cores mais lentos (sem HT). A partir de 8 threads todos os
núcleos físicos já estão ocupados; acrescentar mais threads distribui trabalho entre P-cores
já sobrecarregados via HT e não agrega velocidade, o que explica a leve queda de speedup de
3.19x (8 threads) para 3.16x (10 threads). A eficiência cai de 75% com 2 threads até 32%
com 10 threads, refletindo esse esgotamento progressivo do paralelismo útil.
