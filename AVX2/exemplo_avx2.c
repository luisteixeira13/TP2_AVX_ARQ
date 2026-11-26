#include <immintrin.h>
#include <inttypes.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

// O produto uint32_t e definido modulo 2^32, inclusive em caso de overflow.
static __attribute__((noipa)) void escalar(const uint32_t *a, const uint32_t *b,
                                         uint32_t *out, size_t n) {
    for (size_t i = 0; i < n; ++i)
        out[i] = a[i] * b[i];
}

static __attribute__((noipa)) void avx2(const uint32_t *a, const uint32_t *b,
                                      uint32_t *out, size_t n) {
    size_t i = 0;
    const size_t limite = n - n % 8;
    for (; i < limite; i += 8) {
        // As cargas e a escrita aceitam enderecos sem alinhamento a 32 bytes.
        __m256i va = _mm256_loadu_si256((const __m256i_u *)(a + i));
        __m256i vb = _mm256_loadu_si256((const __m256i_u *)(b + i));
        // VPMULLD devolve os 32 bits baixos de cada um dos oito produtos.
        __m256i produto = _mm256_mullo_epi32(va, vb);
        _mm256_storeu_si256((__m256i_u *)(out + i), produto);
    }
    for (; i < n; ++i)
        out[i] = a[i] * b[i];
}

int main(void) {
    const size_t n = 1000003;
    const int repeticoes = 160;
    uint32_t *a = malloc(n * sizeof(*a));
    uint32_t *b = malloc(n * sizeof(*b));
    uint32_t *ref = malloc(n * sizeof(*ref));
    uint32_t *out = malloc(n * sizeof(*out));
    int status = EXIT_FAILURE;
    if (!a || !b || !ref || !out) {
        fputs("Falha na alocacao.\n", stderr);
        goto liberar;
    }

    for (size_t i = 0; i < n; ++i) {
        a[i] = (uint32_t)(i % 1024 + 1);
        b[i] = (uint32_t)(i % 13 + 2);
    }
    a[0] = UINT32_MAX; // Caso que exercita o truncamento modulo 2^32.

    escalar(a, b, ref, n);
    avx2(a, b, out, n);
    clock_t inicio = clock();
    for (int r = 0; r < repeticoes; ++r)
        escalar(a, b, ref, n);
    clock_t meio = clock();
    for (int r = 0; r < repeticoes; ++r)
        avx2(a, b, out, n);
    clock_t fim = clock();

    for (size_t i = 0; i < n; ++i) {
        if (ref[i] != out[i]) {
            fprintf(stderr, "Divergencia no indice %zu.\n", i);
            goto liberar;
        }
    }

    const double t_escalar = (double)(meio - inicio) / CLOCKS_PER_SEC;
    const double t_avx2 = (double)(fim - meio) / CLOCKS_PER_SEC;
    printf("AVX2 | _mm256_mullo_epi32 | %" PRIu32 " elementos de borda\n", (uint32_t)(n % 8));
    printf("N = %zu | repeticoes = %d | validacao exata: OK\n", n, repeticoes);
    printf("Amostra [0]: %" PRIu32 " x %" PRIu32 " -> %" PRIu32 "\n", a[0], b[0], out[0]);
    printf("Amostra [%zu]: %" PRIu32 " x %" PRIu32 " -> %" PRIu32 "\n",
           n - 1, a[n - 1], b[n - 1], out[n - 1]);
    printf("Escalar: %.6f s | AVX2: %.6f s\n", t_escalar, t_avx2);
    if (t_avx2 > 0)
        printf("Razao de tempos (escalar / AVX2): %.2fx\n", t_escalar / t_avx2);
    status = EXIT_SUCCESS;

liberar:
    free(a);
    free(b);
    free(ref);
    free(out);
    return status;
}
