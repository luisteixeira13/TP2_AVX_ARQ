# Trabalho Prático 2: Paralelismo de Dados e Extensões Vetoriais (AVX/AVX2/FMA)

**Disciplina:** Arquitetura de Computadores II – GCC123/PCC507  
**Universidade Federal de Lavras (UFLA)** – Departamento de Ciência da Computação  
**Professor:** Luiz Henrique A. Correia  

## Grupo 6

- **Bernado Henrique Diniz Felix**
- **Guilherme dos Santos Cordeiro**
- **Luis Felipe Costa Teixeira**

> *Composição do grupo: 3 integrantes, conforme exceção autorizada pelo professor para este grupo (mantida a mesma formação do Trabalho Prático 1).*

---

## Sobre o Trabalho

Este repositório reúne os arquivos e códigos desenvolvidos para o **Trabalho Prático 2**, cujo objetivo é estudar os conceitos de paralelismo no nível de dados em arquiteturas modernas por meio de processadores vetoriais, modelo SIMT e programação em C/C++ utilizando funções intrínsecas SIMD (**AVX**, **AVX2** e **FMA**).

### Estrutura e Andamento Atual

O trabalho está dividido em duas etapas principais:

- [x] **Etapa 1 – Resumo Teórico (`Resumo.md`):** Levantamento conceitual baseado nas aulas gravadas da disciplina, abordando Taxonomia de Flynn, modelo vetorial VMIPS, mecanismos de *Strip Mining* e registrador VLR, programação com intrínsecas AVX/AVX2 em C/C++ e arquitetura de GPUs (modelo SIMT e hierarquia de *threads*, blocos e *grids*).
- [ ] **Etapa 2 – Implementação Prática e Vídeo:** Desenvolvimento de 3 exemplos práticos comentados em C/C++ (utilizando instruções **AVX**, **AVX2** e **FMA**) e gravação de apresentação explicativa de até 15 minutos demonstrando a execução e análise de desempenho.

---

## Execução da Parte AVX no Windows (Ambiente e Passos)

Para executar o exemplo prático de **AVX** no Windows, foram necessários os seguintes passos:

1. **Verificação de suporte SIMD no hardware (CPU-Z):** Conferir no **CPU-Z** (aba *CPU*, campo *Instructions*) se o processador possui suporte a `AVX` (e `AVX2` / `FMA3`) — no teste local, foi utilizado um **Intel Core i9-13900H** com suporte nativo às extensões.
2. **Instalação do compilador GCC via MSYS2:** Instalar o **MSYS2** e, no terminal **UCRT64**, atualizar os pacotes (`pacman -Syu`) e instalar o GCC (`pacman -S --needed base-devel mingw-w64-ucrt-x86_64-gcc`, evitando ponto final no nome do pacote).
3. **Navegação até a pasta do projeto no terminal MSYS2:** Acessar o diretório usando barras normais (`/`) ou aspas, por exemplo:
   
   cd "/c/Users/chand/Desktop/TP2_AVX_ARQ"
   
4. **Atenção ao alinhamento de memória no Windows/MinGW (_aligned_malloc):** Diferentemente do ambiente POSIX/Linux padrão que expõe aligned_alloc diretamente, no ambiente MinGW/UCRT64 a alocação alinhada em 32 bytes exigida pelas cargas alinhadas de 256 bits (`_mm256_load_ps / _mm256_store_ps`) é feita com _aligned_malloc(bytes, 32) e liberada com _aligned_free(...) (via <malloc.h>).

5. **Compilação e execução:** Compilar o código-fonte passando -mavx (e as flags de otimização desejadas) e executar o binário gerado:

  gcc -O2 -mavx exemplo_avx.c -o exemplo_avx.exe && ./exemplo_avx.exe