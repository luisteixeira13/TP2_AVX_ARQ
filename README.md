# TP2 · Paralelismo de dados com AVX, AVX2 e FMA

Arquitetura de Computadores II · GCC123/PCC507 · Universidade Federal de Lavras\
Professor Luiz Henrique A. Correia

**Grupo 6**

- Bernado Henrique Diniz Felix
- Guilherme dos Santos Cordeiro
- Luis Felipe Costa Teixeira

Grupo com três integrantes, conforme a exceção autorizada para manter a formação do Trabalho Prático 1.

## Pasta de entrega

O material para montar e enviar o trabalho está em [entrega](entrega/).

| Arquivo | Uso |
|---|---|
| [Resumo escrito em PDF](entrega/Resumo_TP2_Grupo_6.pdf) | Anexar no Campus Virtual. |
| [Código-fonte comentado em ZIP](entrega/Fontes_TP2_Grupo_6.zip) | Rodar os três exemplos na gravação e anexar o ZIP no Campus Virtual. |
| [Slides editáveis](entrega/Slides_TP2_Grupo_6.pptx) | Usar na apresentação. |
| [Roteiro em PDF](entrega/Roteiro_TP2_Grupo_6.pdf) | Seguir as falas e os comandos de cada integrante. |
| [Bernado · Parte 1](entrega/apresentacaobernadoparte1.mp4) e [Parte 2](entrega/apresentacaobernadoparte2.mp4) | Inserir nessa ordem na montagem. |

Depois das duas partes do Bernado, entra o Guilherme com AVX2 (slides 6 e 7), seguido do Luis com FMA, resultados e conclusão (slides 8 a 11). O vídeo final deve ter até 15 minutos. Enviar o link do YouTube junto com o resumo e o código-fonte no Campus Virtual até 27/11/2026, conforme o enunciado.

## Material de referência

- [Roteiro editável](apresentacao/Roteiro.md) e [PDFs individuais](apresentacao/roteiros/pdf/)
- [Slides em PDF](apresentacao/referencia/Slides_TP2_Grupo_6.pdf) e [MP4 de apoio visual](apresentacao/referencia/Apoio_Visual_TP2_Grupo_6.mp4)
- [Saídas dos testes](apresentacao/Resultados.txt)

## Compilar e executar

Use GCC num sistema x86-64 com suporte a AVX, AVX2 e FMA. No Linux, as extensões aparecem em `/proc/cpuinfo`. No Windows, podem ser conferidas no CPU-Z e os comandos podem ser executados no terminal MSYS2 UCRT64.

Execute a partir da raiz do repositório:

```sh
gcc -std=c11 -O2 -fno-tree-vectorize -mavx AVX/exemplo_avx.c -o avx
./avx

gcc -std=c11 -O2 -fno-tree-vectorize -mavx2 AVX2/exemplo_avx2.c -o avx2
./avx2

gcc -std=c11 -O2 -fno-tree-vectorize -ffp-contract=off -mavx -mfma FMA/exemplo_fma.c -lm -o fma
./fma
```

No Windows, use `avx.exe`, `avx2.exe` e `fma.exe` como nomes dos executáveis tanto na compilação quanto na execução. O exemplo AVX reserva memória alinhada com `aligned_alloc` em Linux e `_aligned_malloc` no Windows. Os exemplos AVX2 e FMA usam intrínsecas de carga e escrita que aceitam endereços desalinhados a 32 bytes.

## Resultados da captura

A execução de referência foi feita em Linux x86-64, num Ryzen 5 3600, com GCC 16.2.1. Os três programas passaram na validação. A razão da tabela divide o tempo escalar pelo vetorial.

| Exemplo | Escalar | Vetorial | Razão |
|---|---:|---:|---:|
| AVX, 1.000.003 elementos | 0,045890 s | 0,023928 s | 1,92× |
| AVX, 8.000.003 elementos | 0,109432 s | 0,106617 s | 1,03× |
| AVX2, 1.000.003 elementos | 0,065025 s | 0,022961 s | 2,83× |
| FMA, 1.000.003 elementos | 0,075102 s | 0,041237 s | 1,82× |

O caso maior da subtração teve ganho menor. O resultado é compatível com o custo de movimentar os dados, já que cada elemento envolve 12 bytes lógicos e uma subtração. A vazão impressa é calculada com esses 12 bytes por elemento, sem medição direta do tráfego da DRAM. O FMA compara o vetor com `fmaf` escalar. Um teste separado mostra zero com produto e soma arredondados separadamente e −2⁻⁴⁶ com a operação fundida.

Os números são de uma rodada por cenário. As chamadas foram aquecidas antes da medição, os tempos no Linux são de CPU via `clock()`, e a autovetorização foi desativada para a comparação com os laços escalares. A [análise inicial de AVX em Windows](AVX/AnaliseAVX.md) registra uma rodada anterior, com outra máquina e metodologia.
