# Bernado · Abertura, SIMD e AVX

**Bernado Henrique Diniz Felix · 00:00–02:45 · Slides 1 a 5**

Leia as falas entre aspas. As linhas em itálico são ações para a gravação.

## Slide 1 · 00:00–00:20

> “Olá, eu sou o Bernado, do Grupo 6. Preparamos três exemplos em C para mostrar como o processador aplica uma operação a vários elementos de um vetor. Para entender os três, vamos começar pelo modelo SIMD.”

## Slide 2 · 00:20–00:55

*Aponte para a primeira coluna e percorra as outras com o cursor.*

> “SIMD significa uma instrução para vários dados. O diagrama mostra como isso funciona numa subtração. Na primeira coluna, subtraímos o primeiro valor de B do primeiro valor de A. A mesma conta vale para as outras sete colunas. Como cada número tem 32 bits, cabem oito num registrador de 256 bits. Assim, uma única instrução faz as oito subtrações.”

## Slide 3 · 00:55–01:30

> “Nos nossos exemplos, esse paralelismo aparece de três formas. Para a subtração de valores de ponto flutuante, usamos AVX. O AVX2, que aparece no segundo exemplo, amplia as operações com inteiros em vetores de 256 bits. No terceiro, o FMA combina uma multiplicação e uma soma, com um único arredondamento no final. Vamos ver como a subtração aparece no código.”

## Slide 4 · 01:30–02:05

*Mostre as duas cargas, a linha com `_mm256_sub_ps` e a linha que grava a resposta.*

> “No código, o bloco de oito que vimos no diagrama começa nestas duas linhas. Cada uma carrega oito valores de um vetor. A linha que eu destaquei faz as subtrações. A última linha grava os oito resultados. Como escolhemos cargas alinhadas, o começo de cada bloco precisa estar numa posição de memória múltipla de 32 bytes. Depois do último bloco completo, sobram três elementos. Um laço separado calcula esses três resultados, um por vez.”

## Slide 5 · 02:05–02:45

> “Agora vamos executar o mesmo código com dois tamanhos de vetor e comparar os tempos.”

*Mostre o terminal e rode. No Windows, troque `avx` por `avx.exe` nos dois comandos.*

```sh
gcc -std=c11 -O2 -fno-tree-vectorize -mavx AVX/exemplo_avx.c -o avx
./avx
```

> “O OK aqui indica que as respostas bateram com as do laço que calcula uma posição por vez. Na medição do slide, a versão AVX levou cerca de metade do tempo no vetor menor. No maior, os tempos ficaram próximos. Para cada subtração, o programa precisa ler dois valores e gravar um resultado. Esse movimento de dados também pesa no tempo. Agora passo a palavra ao Guilherme, que vai mostrar a versão com inteiros.”
