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

---

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

---

# Luis · FMA, medições e conclusão

**Luis Felipe Costa Teixeira · 04:00–06:50 · Slides 8 a 11**

Leia as falas entre aspas. As linhas em itálico são ações para a gravação.

## Slide 8 · 04:00–04:35

*Aponte para os três valores carregados e depois para `_mm256_fmadd_ps`.*

> “Eu sou o Luis. No exemplo anterior, multiplicamos inteiros. Agora voltamos ao ponto flutuante para mostrar o FMA. Ele combina uma multiplicação e uma soma. Em cada posição, multiplicamos A por B e somamos C. A linha destacada faz essa conta em oito posições de uma vez, com um arredondamento por resultado. A versão que calcula uma posição por vez também usa a operação fundida. Assim, comparamos os tempos de duas versões que seguem a mesma regra de arredondamento.”

## Slide 9 · 04:35–05:15

> “Vou rodar o exemplo. A primeira parte da saída valida os dois laços. Logo abaixo, uma conta curta mostra o efeito desse arredondamento único.”

*Rode o programa e destaque as linhas da conta separada e da fundida. No Windows, use `fma.exe`.*

```sh
gcc -std=c11 -O2 -fno-tree-vectorize -ffp-contract=off -mavx -mfma \
    FMA/exemplo_fma.c -lm -o fma
./fma
```

> “Multiplicamos um valor um pouco acima de um por outro um pouco abaixo de um. Depois, subtraímos um. Se arredondamos o produto primeiro, ele fica igual a um e o resultado final é zero. Com FMA, a subtração entra antes do arredondamento. O resultado é o valor negativo, bem pequeno, que aparece na última linha.”

## Slide 10 · 05:15–06:05

*Aponte para as duas barras de AVX e depois para as de AVX2 e FMA.*

> “Com os resultados dos três programas validados, podemos comparar os tempos. Cada barra mostra o tempo escalar dividido pelo tempo da versão vetorial. Na rodada que está aqui, o AVX marcou 1,92 no vetor menor e 1,03 no maior. A conta é a mesma. No vetor maior, há mais dados para carregar e gravar, e a vantagem diminuiu. O AVX2 marcou 2,83, e o FMA vetorial marcou 1,82 em relação ao FMA escalar. Os resultados são de uma rodada no mesmo Ryzen, com a vetorização automática desativada. Em outra máquina ou com outro tamanho de vetor, os tempos podem mudar.”

## Slide 11 · 06:05–06:50

> “Os testes de correção ajudaram a entender o que cada operação faz com os dados. No AVX, conferimos os elementos que ficam depois do último bloco. No AVX2, testamos um produto que ultrapassa 32 bits. No FMA, vimos que o arredondamento intermediário pode eliminar uma diferença pequena. A subtração também mostrou que o ganho depende do volume de dados que o processador precisa movimentar. Obrigado pela atenção.”
