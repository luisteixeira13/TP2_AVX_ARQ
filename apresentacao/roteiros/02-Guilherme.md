# Guilherme · Produto de inteiros com AVX2

**Guilherme dos Santos Cordeiro · 02:45–04:00 · Slides 6 e 7**

Leia as falas entre aspas. As linhas em itálico são ações para a gravação.

## Slide 6 · 02:45–03:20

*Mostre os vetores de inteiros no código e aponte para `_mm256_mullo_epi32`.*

> “Eu sou o Guilherme. No exemplo do Bernado, o AVX subtraía oito valores de ponto flutuante. Agora usamos AVX2 para multiplicar oito pares de inteiros sem sinal. Esta é a linha que faz os produtos. Cada posição da saída guarda os 32 bits menos significativos do produto. O laço escalar usa o mesmo tipo de inteiro. Assim, as duas versões seguem a mesma regra quando o produto ultrapassa o limite de 32 bits.”

## Slide 7 · 03:20–04:00

> “Vamos testar essa regra na prática. Reparem na primeira e na última amostra.”

*Mostre o terminal e rode. No Windows, troque `avx2` por `avx2.exe` nos dois comandos.*

```sh
gcc -std=c11 -O2 -fno-tree-vectorize -mavx2 AVX2/exemplo_avx2.c -o avx2
./avx2
```

> “Na primeira amostra, multiplicamos por dois o maior inteiro sem sinal de 32 bits. A saída mostra o resultado depois do truncamento. A última amostra é calculada fora do bloco vetorial. Nela, 579 vezes 5 dá 2.895. A validação confirma que as duas versões concordaram em todas as posições. Agora passo a palavra ao Luis, que vai explicar o arredondamento no FMA.”
