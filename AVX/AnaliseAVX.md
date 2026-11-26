# Análise Experimental e Teórica — Extensão Vetorial AVX

Este documento consolida os testes práticos, as métricas de desempenho e a análise crítica referentes ao **Exemplo Prático 1 (AVX — 256 bits)** do Trabalho Prático 2, servindo de base para a apresentação em vídeo (itens **c**, **d** e **e** do enunciado).

---

## 1. Ambiente de Execução e Especificações de Hardware

Os testes foram executados localmente em ambiente Windows 11 utilizando o compilador GCC (MSYS2 UCRT64) com suporte nativo às extensões SIMD da arquitetura x86-64:

* **Processador (CPU):** Intel Core i9-13900H (13ª Geração, codinome *Raptor Lake*, 10 nm)
* **Topologia de Núcleos:** 14 núcleos físicos (6 *P-Cores* de desempenho + 8 *E-Cores* de eficiência) e 20 *threads*
* **Hierarquia de Cache:**
  * **L1 Dados:** 6 × 48 KB + 8 × 32 KB
  * **L1 Instruções:** 6 × 32 KB + 8 × 64 KB
  * **L2:** 6 × 1,25 MB + 2 × 2 MB
  * **L3 (Smart Cache):** 24 MB compartilhados
* **Extensões SIMD suportadas (verificadas via CPU-Z):** MMX, SSE, SSE2, SSE3, SSSE3, SSE4.1, SSE4.2, **AVX**, **AVX2**, AVX-VNNI e **FMA3**
* **Compilador e Flags:** `gcc.exe` 16.2.0 (MSYS2 UCRT64) com `-mavx` (e variações `-O0`, `-O2 -fno-tree-vectorize` e `-O2`)[cite: 31]

---

## 2. Metodologia e Estrutura do Código (`exemplo_avx.c`)

O programa avalia a subtração elemento a elemento de dois vetores de ponto flutuante de precisão simples (`float`, 32 bits), comparando uma implementação escalar tradicional (`out[i] = a[i] - b[i]`) com a versão vetorizada via funções intrínsecas AVX (`<immintrin.h>`)[cite: 33]:

* **Registradores YMM de 256 bits (`__m256`):** Cada registrador armazena simultaneamente **8 valores `float`** ($8 \times 32\text{ bits} = 256\text{ bits}$), reduzindo o número de iterações do laço principal em **$8\times$** (`i += 8`).
* **Intrínsecas utilizadas:**
  * `_mm256_load_ps`: carrega 8 *floats* contíguos da memória alinhada em 32 bytes para um registrador YMM.
  * `_mm256_sub_ps`: executa 8 subtrações em ponto flutuante simultaneamente em uma única instrução SIMD (`vsubps`).
  * `_mm256_store_ps`: grava os 8 resultados do registrador YMM de volta na memória alinhada.
* **Alinhamento e tratamento de borda:** Os vetores são alocados com `_aligned_malloc(..., 32)` para atender ao requisito de alinhamento de 32 bytes das cargas/escritas alinhadas, e o código inclui um laço escalar residual ao final para tratar vetores cujo tamanho $N$ não seja múltiplo exato de 8.
* **Cenários avaliados (Hierarquia de Memória / Modelo Roofline):**
  1. **Dentro da Cache L3:** $N = 1.000.000$ elementos (`3,81 MB` por vetor; **`11,44 MB` totais** para `a`, `b` e `out`), cabendo integralmente nos **24 MB de Cache L3** do i9-13900H (1.600 repetições).
  2. **Limitado pela DRAM (fora da L3):** $N = 8.000.000$ elementos (`30,52 MB` por vetor; **`91,55 MB` totais**), excedendo a Cache L3 e forçando transferências contínuas com a memória principal (200 repetições)[cite: 31, 33].

---

## 3. Resultados Experimentais Coletados

Em todos os cenários, a validação numérica entre o vetor escalar e o vetor AVX foi aprovada (`OK`). Como cada subtração realiza **1 FLOP** e movimenta **12 bytes** (leitura de 4 B em `a`, leitura de 4 B em `b` e escrita de 4 B em `out`), a intensidade aritmética do algoritmo é constante e igual a **$\frac{1}{12} \approx 0,0833\text{ FLOP/byte}$**[cite: 33].

| Cenário de Memória | Flags de Compilação | Tempo Escalar (s) | Tempo AVX (s) | Desempenho AVX | Largura de Banda AVX | Speedup (`Escalar / AVX`) |
| :--- | :--- | :---: | :---: | :---: | :---: | :---: |
| **Dentro da Cache L3** (`11,44 MB`) | `-O0 -mavx` | `1,514000 s` | `0,762000 s` | `2,10 GFLOP/s` | `25,20 GB/s` | **1,99x** |
| **Excede Cache L3 / DRAM** (`91,55 MB`) | `-O0 -mavx` | `1,571000 s` | `0,960000 s` | `1,67 GFLOP/s` | `20,00 GB/s` | **1,64x** |
| **Dentro da Cache L3** (`11,44 MB`) | `-O2 -fno-tree-vectorize -mavx` | `0,515000 s` | `0,314000 s` | `5,10 GFLOP/s` | `61,15 GB/s` | **1,64x** |
| **Excede Cache L3 / DRAM** (`91,55 MB`) | `-O2 -fno-tree-vectorize -mavx` | `0,832000 s` | `0,793000 s` | `2,02 GFLOP/s` | `24,21 GB/s` | **1,05x** |
| **Dentro da Cache L3** (`11,44 MB`) | `-O2 -mavx` | `0,506000 s` | `0,308000 s` | `5,19 GFLOP/s` | `62,34 GB/s` | **1,64x** |
| **Excede Cache L3 / DRAM** (`91,55 MB`) | `-O2 -mavx` | `0,903000 s` | `0,783000 s` | `2,04 GFLOP/s` | `24,52 GB/s` | **1,15x** |

---

## 4. Análise dos Benefícios das Funções Intrínsecas (Item d)

1. **Exploração direta do paralelismo no nível de dados (SIMD):** As intrínsecas AVX permitem processar 8 operandos `float` de 32 bits simultaneamente em registradores de 256 bits com uma única instrução aritmética, reduzindo em $8\times$ a quantidade de instruções de controle de laço (*overhead* de incremento e desvios condicionais) e o número total de instruções buscadas e decodificadas pelo processador[cite: 1, 33].
2. **Ganho expressivo de vazão e desempenho na hierarquia de cache:** Nos testes dentro da Cache L3, o código vetorizado elevou a taxa de processamento de `3,16 GFLOP/s` para **`5,19 GFLOP/s`** (com `-O2`) e atingiu **`62,34 GB/s`** de largura de banda efetiva, alcançando *speedup* de **`1,64x`** mesmo contra o código escalar otimizado e **`1,99x`** sem otimização (`-O0`).
3. **Controle explícito e previsibilidade sem depender de Assembly puro:** Diferentemente da autovetorização automática do compilador — que pode falhar diante de ponteiros com possível *aliasing*, condições internas ou estruturas de dados complexas —, o uso de intrínsecas garante determinismo na geração das instruções vetoriais (`vmovaps`, `vsubps`) mantendo a legibilidade e a portabilidade da linguagem C[cite: 1, 4, 19].

---

## 5. Conclusão: Aprendizados e Limitações Observadas (Item e)

* **O *speedup* real não atinge o limite teórico de $8\times$ devido ao gargalo de memória (Modelo Roofline):** Embora um registrador de 256 bits processe 8 *floats* por instrução, a subtração vetorial possui intensidade aritmética muito baixa ($0,0833\text{ FLOP/byte}$). Como a operação é fortemente limitada pela memória (*memory-bound*), quando os dados ultrapassam os 24 MB da Cache L3 (`91,55 MB` totais) e passam a depender da DRAM, a largura de banda cai de `62,34 GB/s` para `24,52 GB/s` e o *speedup* estabiliza em `1,15x` (com `-O2`) e `1,64x` (com `-O0`). Isso comprova na prática o Modelo Roofline estudado em aula: sem reuso de dados em cache ou maior intensidade aritmética (como em multiplicação de matrizes ou operações FMA), a ULA vetorial fica ociosa aguardando transferências da memória[cite: 1, 31, 33].
* **Impacto das otimizações do compilador (`-O2` vs. `-O0`):** Ao habilitar `-O2 -mavx`, o GCC moderno aplica autovetorização e desenrolamento de laço (*loop unrolling*) também sobre a função escalar simples, aproximando seu tempo (`0,506 s` na L3) do código com intrínsecas (`0,308 s`), enquanto em `-O0` a diferença aparece de forma isolada (`1,514 s` contra `0,762 s`).
* **Restrições de alinhamento, arquitetura e complexidade de programação:** Instruções alinhadas como `_mm256_load_ps` exigem endereços de memória alinhados em fronteiras de 32 bytes (sob risco de falha de segmentação caso não alinhados, exigindo `_mm256_loadu_ps`)[cite: 9]. Além disso, há dependência das *flags* suportadas pelo hardware onde o binário será executado (ex.: evitar AVX-512 em máquinas que suportam apenas AVX/AVX2) e maior esforço de desenvolvimento para lidar com tamanhos de vetores não múltiplos da largura SIMD e desvios condicionais (que exigem máscaras vetoriais)[cite: 1, 33].