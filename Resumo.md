# Resumo das Aulas – Arquiteturas Vetoriais e Paralelismo de Dados
**Disciplina:** GCC123 – Arquitetura de Computadores II  
**Trabalho Prático 2 – Etapa 1**

---

## 1. Taxonomia de Flynn (SISD, SIMD, MISD e MIMD)

A Taxonomia de Flynn, proposta por Michael Flynn em 1966, classifica as arquiteturas de computadores observando dois fatores principais: o fluxo de instruções que entram para serem processadas e o fluxo de dados sobre os quais essas instruções atuam. A partir da combinação desses dois fluxos, surgem quatro categorias principais:

* **SISD (*Single Instruction, Single Data*):** Representa os computadores sequenciais tradicionais (como os PCs antigos monotarefa). Uma única unidade de controle busca um fluxo único de instruções na memória e direciona um único elemento de processamento (PE) para operar sobre um único fluxo de dados por vez. Não há exploração de paralelismo nem no nível de instruções nem no nível de dados.
* **SIMD (*Single Instruction, Multiple Data*):** Também chamados de processadores de *arrays* ou vetoriais. Nesse modelo, uma única instrução é aplicada simultaneamente sobre vários conjuntos de dados diferentes. A execução pode acontecer de forma sequencial em *pipeline* ou em paralelo por múltiplas unidades funcionais. Uma vantagem desse modelo é que o programador ainda consegue raciocinar e estruturar o código de forma sequencial, enquanto o hardware realiza as operações em paralelo sobre os vetores de dados (além de ser mais eficiente energeticamente que o MIMD). Atualmente, essa categoria inclui a subdivisão **SIMT** (*Single Instruction, Multiple Threads*), muito empregada em GPUs.
* **MISD (*Multiple Instruction, Single Data*):** Vários fluxos de instruções diferentes operam simultaneamente sobre o mesmo fluxo de dados, devendo concordar quanto aos resultados gerados. Embora literaturas antigas a tratassem como pouco implementável, na prática ela é utilizada em sistemas críticos que exigem **tolerância a falhas** por meio de redundância — como computadores de bordo de aviões e sistemas de controle de voo de ônibus espaciais, onde múltiplos sistemas redundantes processam os mesmos dados para evitar falhas catastróficas.
* **MIMD (*Multiple Instruction, Multiple Data*):** Consiste em múltiplos processadores autônomos executando instruções distintas sobre dados distintos ao mesmo tempo. É a base dos processadores superescalares, arquiteturas *multi-core* e sistemas distribuídos, podendo operar tanto com espaço de memória compartilhada quanto com memória distribuída.

---

## 2. Modelo VMIPS

Para estudar o funcionamento interno de um processador vetorial puro, utiliza-se o **VMIPS** (MIPS Vetorial), um modelo conceitual proposto por Hennessy e baseado nos supercomputadores da família Cray (como o Cray-1). Diferente de um processador escalar comum, em que cada registrador guarda apenas uma palavra de 32 ou 64 bits, o VMIPS é projetado para carregar blocos inteiros de dados da memória, processá-los em fluxo contínuo e devolvê-los à memória.

Na organização de hardware do VMIPS, o banco principal é composto por **8 registradores vetoriais** (`V0` a `V7`), sendo que cada registrador funciona como um *array* capaz de armazenar **64 elementos de 64 bits** cada (precisão simples ou dupla). Para sustentar o fluxo simultâneo de operandos, esse banco conta com 16 portas de leitura e 8 portas de escrita, além de manter os 32 registradores escalares de inteiros e os 32 de ponto flutuante do MIPS tradicional. Tudo isso é interligado às unidades funcionais (soma/subtração, multiplicação, divisão, inteiros e unidade lógica) e à unidade de *load/store* por meio de chaves do tipo *crossbar*, todas totalmente implementadas em *pipeline*.

No conjunto de instruções do VMIPS, as operações vetoriais recebem sufixos que indicam os tipos de operandos: `.VV` quando operam dois vetores (como `ADDVV.D V1, V2, V3`) e `.VS` quando combinam um vetor com um escalar (como `MULVS.D V2, V1, F0`, onde o valor escalar de `F0` é multiplicado por cada um dos 64 elementos de `V1`). Como exemplo prático visto em aula, o cálculo **DAXPY** ($Y[i] = a \times X[i] + Y[i]$) para 64 elementos exige 578 instruções no MIPS escalar devido ao controle de laço, mas cai para apenas 6 instruções no VMIPS (`L.D`, `LV`, `MULVS.D`, `ADDVV.D`, `LV` e `SV`).

Quanto ao desempenho e controle de conflitos no VMIPS, destacam-se três conceitos fundamentais:
* **Ocultação de latência e *Chaining* (encadeamento):** Enquanto no MIPS escalar um conflito de dados (RAW) gera bolhas a cada iteração do laço, no VMIPS a penalidade de latência inicial (como os 12 ciclos de *startup* de um *load* vetorial ou 6 ciclos da soma em ponto flutuante) ocorre apenas uma vez para o primeiro elemento do vetor; os elementos seguintes fluem continuamente pelo *pipeline* a uma taxa de 1 elemento por ciclo. Com o *chaining*, uma instrução dependente (como um `MULVS.D` logo após um `LV`) não precisa esperar os 64 elementos terminarem de carregar: assim que o primeiro elemento fica pronto, ele já é encaminhado para a próxima unidade funcional.
* **Comboios (*Convoys*) e *Chimes*:** Um *convoy* é o conjunto de instruções vetoriais que podem ser executadas juntas sem provocar conflitos estruturais ou de dados. O tempo necessário para executar um *convoy* completo é chamado de *chime*. Se uma sequência de código precisa ser dividida em $M$ comboios (por exemplo, quando há dois *loads* vetoriais disputando a única unidade de *load/store*, gerando hazard estrutural) para processar um vetor de $N$ elementos, o tempo total de execução será aproximadamente $M \times N$ ciclos de clock.
* **Múltiplas Pistas (*Lanes*):** Para ir além de 1 elemento concluído por ciclo, a arquitetura divide os registradores vetoriais e as unidades funcionais em múltiplas pistas paralelas. Em um sistema com 4 *lanes*, por exemplo, os elementos de índices `0, 4, 8...` vão para a pista 0; `1, 5, 9...` para a pista 1; `2, 6, 10...` para a pista 2; e `3, 7, 11...` para a pista 3, permitindo concluir 4 operações por ciclo de clock.

---

## 3. Mecanismos *Strip Mining* e Registrador de Tamanho Vetorial (VLR)

Os registradores vetoriais do VMIPS possuem um limite físico fixo de elementos que conseguem armazenar de uma só vez, chamado de **MVL** (*Maximum Vector Length*, que no VMIPS vale 64). Na prática, porém, os vetores e matrizes das aplicações raramente têm exatamente 64 elementos — podem ter menos (ex.: 9 elementos) ou muito mais (ex.: 73 elementos ou uma matriz $100 \times 100$).

Para lidar com vetores de tamanho arbitrário $N$ sem estourar o limite dos registradores nem invadir posições indevidas de memória, a arquitetura utiliza dois recursos trabalhando juntos:

1. **Registrador de Comprimento Vetorial (VLR – *Vector Length Register*):** É um registrador especial (alocado no espaço do coprocessador 1 do MIPS) que armazena o número exato de elementos válidos que devem ser processados pela instrução vetorial atual. Dessa forma, sempre que o comprimento do trecho processado for menor que o `MVL` (64), o hardware restringe a execução apenas aos elementos definidos pelo VLR.
2. ***Strip Mining*:** É a técnica de particionamento de laços responsável por fatiar um vetor de tamanho $N$ qualquer em blocos menores que caibam nos registradores vetoriais.

O funcionamento do *strip mining* pode ser acompanhado pelo algoritmo analisado em aula para um vetor de tamanho $N = 73$ com $MVL = 64$. Primeiro, calcula-se o resto da divisão inteira para determinar o tamanho do trecho residual: $VL = N \bmod MVL = 73 \bmod 64 = 9$. O registrador VLR é configurado com `9`, e o laço interno processa apenas os primeiros 9 elementos (índices `0` a `8`), evitando que sobre um pedaço incompleto no final. Terminada essa primeira fatia, o algoritmo atualiza o índice base (`low = 9`) e redefine o comprimento do vetor para o tamanho máximo suportado pelo registrador ($VL = MVL = 64$). Na segunda passagem do laço externo, o hardware processa em bloco cheio os 64 elementos restantes (índices `9` a `72`). Se $N$ fosse ainda maior, esse segundo passo se repetiria em fatias de 64 até cobrir todo o vetor.

*(Nota complementar das aulas sobre controle de fluxo e memória no VMIPS: além do VLR, o modelo utiliza o **Registrador de Máscara Vetorial (VM)** para lidar com comandos condicionais `if` dentro de laços — onde uma comparação elemento a elemento preenche um vetor booleano de bits 0 ou 1 que habilita ou inibe a escrita do resultado —, além de técnicas de acesso com passo (**stride**) para percorrer colunas de matrizes sem gerar conflitos nos bancos de memória, e **scatter-gather** para compactar e operar matrizes esparsas usando vetores de índices).*

---

## 4. Conceitos de Programação AVX/AVX2 em C/C++ e Funções Intrínsecas

Nos processadores modernos da família x86 (Intel e AMD), o paralelismo no nível de dados foi incorporado por meio das extensões SIMD. Essa evolução começou com o **MMX** (1996, reutilizando registradores de 64 bits para áudio), passou pelo **SSE** (1999, introduzindo registradores dedicados de 128 bits), chegou ao **AVX** (*Advanced Vector Extensions*, 2010), expandiu-se no **AVX2** (2013) e atingiu 512 bits nas gerações mais novas com o AVX-512. O **AVX/AVX2** dobra a largura dos registradores vetoriais para **256 bits**, permitindo empacotar e operar simultaneamente 4 valores de ponto flutuante de precisão dupla (`double`, 64 bits), 8 valores de precisão simples (`float`, 32 bits) ou múltiplos inteiros (32 de 8 bits, 16 de 16 bits, 8 de 32 bits ou 4 de 64 bits).

Antes de programar, é necessário verificar se o processador da máquina suporta essas instruções (no Linux, checando as *flags* `avx`, `avx2` e `fma` em `/proc/cpuinfo`, ou via CPU-Z no Windows). Para utilizar esses registradores em C/C++ sem precisar escrever código Assembly puro, empregam-se **funções intrínsecas**, que são funções especiais mapeadas diretamente pelo compilador para as instruções vetoriais da CPU.

Na estruturação e compilação de um programa com intrínsecas AVX/AVX2:
* **Cabeçalho e Compilação:** Deve-se incluir a biblioteca `#include <immintrin.h>`. Ao compilar com o GCC, é obrigatório passar as *flags* correspondentes à arquitetura utilizada (`-mavx`, `-mavx2` e `-mfma` quando houver instruções *Fused Multiply-Add*), caso contrário o compilador rejeita as intrínsecas.
* **Tipos de Dados Vetoriais:** Começam sempre com dois *underlines* (`__m`), seguidos pela largura em bits e pelo sufixo do tipo: `__m128`, `__m128d` e `__m128i` (128 bits); e `__m256`, `__m256d` e `__m256i` (256 bits, representando *floats*, *doubles* e inteiros, respectivamente).
* **Convenção de Nomes das Funções:** Começam com um *underline* (`_mm`) e seguem o formato `_mm<largura>_<operação>_<tipo>`. Por exemplo, `_mm256_add_ps` soma dois vetores de 256 bits contendo 8 *floats* empacotados (*packed single-precision*), enquanto `_mm256_sub_pd` subtrai 4 *doubles* (*packed double-precision*). Para inteiros, usam-se sufixos como `epi32` (inteiros com sinal de 32 bits) ou `epu16` (inteiros sem sinal de 16 bits).
* **Principais Categorias de Intrínsecas:**
  1. **Inicialização e Carregamento:** Funções como `_mm256_setzero_pd` (zera o vetor) e `_mm256_setr_ps` / `_mm256_setr_epi32` inicializam os registradores a partir de escalares. O sufixo `r` (*reverse*) é muito utilizado porque a arquitetura x86 é *little-endian* (armazenando o elemento menos significativo à direita); o `setr` permite escrever os valores na ordem direta de leitura. Para ler vetores da memória principal, usa-se `_mm256_load_ps` (exige endereço alinhado em fronteira de 32 bytes, sob pena de *segmentation fault*), `_mm256_loadu_ps` (para memória não alinhada) e `_mm256_maskload_epi32` (do AVX2, que carrega seletivamente elementos conforme o bit mais significativo/sinal de um vetor de máscara).
  2. **Operações Aritméticas e FMA:** Além de soma, subtração, multiplicação e divisão vetorial, o AVX2 oferece operações com saturação (`adds`/`subs`, que travam o resultado no valor mínimo ou máximo do tipo em vez de causar *overflow*), operações horizontais (`hadd`/`hsub`, que somam ou subtraem elementos adjacentes dentro do mesmo vetor) e as instruções **FMA** (*Fused Multiply-Add*, como `_mm256_fmadd_pd`), que executam $a \times b + c$ em uma única etapa com apenas um arredondamento final, aumentando tanto a velocidade quanto a precisão numérica.
  3. **Permutação e Embaralhamento:** Instruções como `_mm256_permute_ps` e `_mm256_shuffle_ps` reorganizam elementos dentro de um vetor ou combinam partes de dois vetores usando máscaras de bits de controle, recurso essencial para implementar operações como multiplicação de números complexos de forma vetorizada.

---

## 5. Arquitetura de GPUs, Modelo SIMT e Hierarquia de Execução

As GPUs (*Graphics Processing Units*) surgiram voltadas ao processamento gráfico intenso, mas evoluíram para aceleradores de propósito geral em problemas massivamente paralelos. Elas operam sob um **modelo de computação heterogênea**, no qual a CPU atua como o **host** (responsável pelo fluxo principal da aplicação, sistema operacional e despacho de tarefas) e a GPU atua como o **device** (coprocessador de alto desempenho conectado via barramento ao sistema). Para programar nesse modelo, utilizam-se plataformas como **CUDA** (*Compute Unified Device Architecture*), uma extensão baseada em C/C++.

Diferente das extensões vetoriais das CPUs (como o AVX), as GPUs adotam o paradigma **SIMT (*Single Instruction, Multiple Threads*)**. Enquanto uma CPU tradicional depende de grandes memórias *cache* e lógica complexa de controle para reduzir a latência de poucas *threads*, a GPU dedica a maior parte de seus transistores a centenas ou milhares de unidades aritméticas (como os 6.912 núcleos de uma NVIDIA A100), ocultando a latência da memória por meio de ***multithreading* massivo em hardware**. Quando um grupo de *threads* aguarda um dado da memória, o escalonador da GPU alterna instantaneamente para outro grupo pronto para executar. Internamente, seus multiprocessadores (processadores SIMD *multithreaded*) organizam pipelines paralelos (*lanes*) com unidades de ponto flutuante, unidades de inteiros, unidades de funções especiais (SFUs, para raízes quadradas, senos e cossenos), unidades de *load/store* (com suporte a *scatter-gather* e máscaras) e núcleos especializados como *Tensor Cores*.

Para mapear um problema paralelo no hardware da GPU, o modelo CUDA organiza a execução em uma hierarquia de três níveis:

```
Grid (Problema completo, ex.: 16 blocos)
 └── Bloco 0, Bloco 1, ..., Bloco 15 (Agrupamentos de threads no mesmo multiprocessador)
      └── Thread 0, Thread 1, ..., Thread 511 (Menor unidade de execução sobre os dados)
```

* ***Thread*:** É a unidade básica de execução. Cada *thread* recebe identificadores próprios e é associada a um elemento individual (ou pequeno subconjunto de elementos) do vetor ou matriz a ser processado.
* **Bloco (*Block*):** É um conjunto de *threads* agrupadas que executam juntas dentro de um mesmo processador SIMD *multithreaded* da GPU. As *threads* de um mesmo bloco podem sincronizar e compartilhar dados rapidamente através de memória local compartilhada.
* ***Grid*:** É o nível mais alto da hierarquia, composto por todos os blocos disparados pela CPU para resolver o domínio inteiro da aplicação. Por exemplo, no processamento de um vetor de 8.192 elementos, pode-se configurar uma *grid* de 16 blocos contendo 512 *threads* cada (invocada em CUDA pela sintaxe `funcao<<<dimGrid, dimBlock>>>(...)`). Dentro do código executado na GPU (*kernel*), cada *thread* calcula o índice global do dado que vai processar combinando a identificação de seu bloco, a dimensão do bloco e sua própria identificação (`i = blockIdx.x * blockDim.x + threadIdx.x`).