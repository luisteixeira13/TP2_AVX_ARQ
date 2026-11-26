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
