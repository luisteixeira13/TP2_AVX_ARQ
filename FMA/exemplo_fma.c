#include <immintrin.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

// fmaf garante a mesma semantica de arredondamento da versao vetorial.
static __attribute__((noipa)) void escalar(const float *a, const float *b, const float *c,
                                         float *out, size_t n) {
    for (size_t i = 0; i < n; ++i)
        out[i] = fmaf(a[i], b[i], c[i]);
}

static __attribute__((noipa)) void vetorial(const float *a, const float *b, const float *c,
                                          float *out, size_t n) {
    size_t i = 0;
    const size_t limite = n - n % 8;
    for (; i < limite; i += 8) {
        __m256 va = _mm256_loadu_ps(a + i);
        __m256 vb = _mm256_loadu_ps(b + i);
        __m256 vc = _mm256_loadu_ps(c + i);
        // A multiplicacao e a soma de cada pista tem um unico arredondamento.
        __m256 resultado = _mm256_fmadd_ps(va, vb, vc);
        _mm256_storeu_ps(out + i, resultado);
    }
    for (; i < n; ++i)
        out[i] = fmaf(a[i], b[i], c[i]);
}

int main(void) {
    const size_t n = 1000003;
    const int repeticoes = 160;
    float *a = malloc(n * sizeof(*a));
    float *b = malloc(n * sizeof(*b));
    float *c = malloc(n * sizeof(*c));
    float *ref = malloc(n * sizeof(*ref));
    float *out = malloc(n * sizeof(*out));
    int status = EXIT_FAILURE;
    if (!a || !b || !c || !ref || !out) {
        fputs("Falha na alocacao.\n", stderr);
        goto liberar;
    }

    for (size_t i = 0; i < n; ++i) {
        a[i] = (float)(i % 97) * 0.1f;
        b[i] = (float)(i % 13 + 1) * 0.3f;
        c[i] = -(float)(i % 31) * 0.25f;
    }
    escalar(a, b, c, ref, n);
    vetorial(a, b, c, out, n);
    clock_t inicio = clock();
    for (int r = 0; r < repeticoes; ++r)
        escalar(a, b, c, ref, n);
    clock_t meio = clock();
    for (int r = 0; r < repeticoes; ++r)
        vetorial(a, b, c, out, n);
    clock_t fim = clock();

    for (size_t i = 0; i < n; ++i) {
        if (ref[i] != out[i]) {
            fprintf(stderr, "Divergencia no indice %zu.\n", i);
            goto liberar;
        }
    }

    // (1 + 2^-23)(1 - 2^-23) - 1. O produto isolado arredonda para 1.
    const float x = 1.0f + 0x1p-23f;
    const float y = 1.0f - 0x1p-23f;
    volatile float produto_arredondado = x * y;
    float separado = produto_arredondado - 1.0f;
    float fundido = fmaf(x, y, -1.0f);
    if (separado != 0.0f || fundido != -0x1p-46f) {
        fputs("Falha no exemplo de arredondamento.\n", stderr);
        goto liberar;
    }

    const double t_escalar = (double)(meio - inicio) / CLOCKS_PER_SEC;
    const double t_fma = (double)(fim - meio) / CLOCKS_PER_SEC;
    printf("FMA | _mm256_fmadd_ps | %zu elementos de borda\n", n % 8);
    printf("N = %zu | repeticoes = %d | fmaf escalar x vetor: OK\n", n, repeticoes);
    printf("Produto e soma separados: %a\n", (double)separado);
    printf("Multiplicacao e soma fundidas: %a (= -2^-46)\n", (double)fundido);
    printf("Escalar fundido: %.6f s | FMA vetorial: %.6f s\n", t_escalar, t_fma);
    if (t_fma > 0)
        printf("Razao de tempos (escalar / vetor): %.2fx\n", t_escalar / t_fma);
    status = EXIT_SUCCESS;

liberar:
    free(a);
    free(b);
    free(c);
    free(ref);
    free(out);
    return status;
}
