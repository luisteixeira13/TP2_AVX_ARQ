#include <immintrin.h> // Funcoes intrinsecas AVX / AVX2 / FMA
#ifdef _WIN32
#include <malloc.h>    // _aligned_malloc e _aligned_free no Windows
#endif
#include <math.h>      // fabsf() para validacao numerica
#include <stdio.h>     // Entrada e saida padrao (printf)
#include <stdlib.h>    // Utilitarios gerais
#include <time.h>      // clock() para medicao de tempo

/*
 * ============================================================================
 * TRABALHO PRATICO 2 - ARQUITETURA DE COMPUTADORES II (GCC123 / PCC507)
 * Exemplo Pratico 1: Extensao Vetorial AVX (256 bits)
 * ============================================================================
 *
 * Objetivo:
 *   Demonstrar o uso de funcoes intrinsecas AVX em C para operar vetores de
 *   ponto flutuante de precisao simples (float, 32 bits) em registradores
 *   YMM de 256 bits (8 floats por instrucao), comparando desempenho, vazao
 *   de memoria (GB/s) e intensidade aritmetica com a implementacao escalar.
 *
 * Intrinsecas AVX utilizadas:
 *   - __m256          : tipo de dado vetorial de 256 bits (8 x float)
 *   - _mm256_load_ps  : carrega 8 floats alinhados em 32 bytes da memoria
 *   - _mm256_sub_ps   : subtrai 8 pares de floats em paralelo (vsubps)
 *   - _mm256_store_ps : grava os 8 floats resultantes na memoria alinhada
 *
 * Compilacao (MSYS2 UCRT64 / GCC):
 *   1) Sem otimizacao (evidencia ganho puro da intrinseca):
 *      gcc -O0 -mavx exemplo_avx.c -o exemplo_avx_o0.exe
 *   2) Com otimizacao -O2 (permite comparar com autovetorizacao do GCC):
 *      gcc -O2 -mavx exemplo_avx.c -o exemplo_avx_o2.exe
 *   3) Com -O2 sem autovetorizacao (isola o escalar puro otimizado):
 *      gcc -O2 -fno-tree-vectorize -mavx exemplo_avx.c -o exemplo_avx_novec.exe
 * ============================================================================
 */

// C11 exige que o tamanho passado a aligned_alloc seja multiplo do alinhamento.
static void *alocar_vetor(size_t bytes) {
#ifdef _WIN32
    return _aligned_malloc(bytes, 32);
#else
    return aligned_alloc(32, (bytes + 31) / 32 * 32);
#endif
}

static void liberar_vetor(void *vetor) {
#ifdef _WIN32
    _aligned_free(vetor);
#else
    free(vetor);
#endif
}

// O atributo do GCC conserva cada chamada medida como uma operacao separada.
// Funcao escalar tradicional: processa 1 elemento por iteracao
static __attribute__((noipa)) void subtracao_escalar(const float *a, const float *b, float *out, int n) {
    for (int i = 0; i < n; i++) {
        out[i] = a[i] - b[i];
    }
}

// Funcao vetorial AVX: processa 8 elementos float por iteracao (256 bits)
static __attribute__((noipa)) void subtracao_avx(const float *a, const float *b, float *out, int n) {
    int limite = n - (n % 8); // Garante multiplos exatos de 8 elementos
    int i = 0;

    for (; i < limite; i += 8) {
        __m256 va = _mm256_load_ps(&a[i]); // Carrega 8 floats de 'a' (32B alinhados)
        __m256 vb = _mm256_load_ps(&b[i]); // Carrega 8 floats de 'b' (32B alinhados)
        __m256 vr = _mm256_sub_ps(va, vb); // Subtrai 8 pares em paralelo
        _mm256_store_ps(&out[i], vr);      // Armazena 8 resultados em 'out'
    }

    // Tratamento escalar de borda caso N nao seja multiplo de 8
    for (; i < n; i++) {
        out[i] = a[i] - b[i];
    }
}

// Retorna o tempo decorrido em segundos
static double tempo_segundos(clock_t inicio, clock_t fim) {
    return (double)(fim - inicio) / (double)CLOCKS_PER_SEC;
}

// Verifica se os vetores escalar e AVX produziram os mesmos resultados
static int validar_resultados(const float *ref, const float *obtido, int n) {
    for (int i = 0; i < n; i++) {
        if (fabsf(ref[i] - obtido[i]) > 1e-5f) {
            return 0;
        }
    }
    return 1;
}

// Executa o benchmark para um tamanho de vetor N e imprime metricas completas
static int executar_benchmark(int n, int repeticoes, const char *cenario) {
    size_t bytes_vetor = (size_t)n * sizeof(float);

    // Alocacao alinhada em 32 bytes (256 bits) exigida por _mm256_load_ps
    float *a = (float *)alocar_vetor(bytes_vetor);
    float *b = (float *)alocar_vetor(bytes_vetor);
    float *res_esc = (float *)alocar_vetor(bytes_vetor);
    float *res_avx = (float *)alocar_vetor(bytes_vetor);

    if (!a || !b || !res_esc || !res_avx) {
        fprintf(stderr, "Erro ao alocar memoria alinhada para N = %d.\n", n);
        liberar_vetor(a);
        liberar_vetor(b);
        liberar_vetor(res_esc);
        liberar_vetor(res_avx);
        return 0;
    }

    // Inicializa os vetores de entrada
    for (int i = 0; i < n; i++) {
        a[i] = (float)(i % 100) * 2.0f;
        b[i] = (float)(i % 100) * 1.0f;
    }

    // Aquecimento (warm-up) para estabilizar caches e frequencia da CPU
    subtracao_escalar(a, b, res_esc, n);
    subtracao_avx(a, b, res_avx, n);

    // Medicao 1: Versao Escalar
    clock_t ini_esc = clock();
    for (int r = 0; r < repeticoes; r++) {
        subtracao_escalar(a, b, res_esc, n);
    }
    clock_t fim_esc = clock();

    // Medicao 2: Versao Vetorial AVX
    clock_t ini_avx = clock();
    for (int r = 0; r < repeticoes; r++) {
        subtracao_avx(a, b, res_avx, n);
    }
    clock_t fim_avx = clock();

    double t_esc = tempo_segundos(ini_esc, fim_esc);
    double t_avx = tempo_segundos(ini_avx, fim_avx);
    double speedup = (t_avx > 0.0) ? (t_esc / t_avx) : 0.0;

    // Calculo de metricas arquiteturais (Modelo Roofline):
    // Cada elemento exige 1 FLOP (subtracao) e movimenta 12 bytes (le A, le B, grava OUT)
    double total_flops = (double)n * (double)repeticoes;
    double total_bytes = (double)n * 3.0 * sizeof(float) * (double)repeticoes;
    double intensidade_aritmetica = total_flops / total_bytes; // 1/12 = 0.0833 FLOP/byte

    double gflops_esc = (t_esc > 0.0) ? (total_flops / t_esc) / 1e9 : 0.0;
    double gflops_avx = (t_avx > 0.0) ? (total_flops / t_avx) / 1e9 : 0.0;
    double bw_esc_gbs = (t_esc > 0.0) ? (total_bytes / t_esc) / 1e9 : 0.0;
    double bw_avx_gbs = (t_avx > 0.0) ? (total_bytes / t_avx) / 1e9 : 0.0;

    int ok = validar_resultados(res_esc, res_avx, n);

    printf("--------------------------------------------------------------------\n");
    printf("Cenario: %s\n", cenario);
    printf("Tamanho N = %d floats (%.2f MiB por vetor | %.2f MiB alocados)\n",
           n, (double)bytes_vetor / (1024.0 * 1024.0),
           (4.0 * (double)bytes_vetor) / (1024.0 * 1024.0));
    printf("Repeticoes: %d | Validacao numerica: %s\n", repeticoes, ok ? "OK" : "ERRO");
    printf("Intensidade Aritmetica: %.4f FLOP/byte (1 FLOP / 12 bytes)\n", intensidade_aritmetica);
    printf("Escalar -> Tempo: %.6f s | Desempenho: %.2f GFLOP/s | Vazao logica: %.2f GB/s\n",
           t_esc, gflops_esc, bw_esc_gbs);
    printf("AVX     -> Tempo: %.6f s | Desempenho: %.2f GFLOP/s | Vazao logica: %.2f GB/s\n",
           t_avx, gflops_avx, bw_avx_gbs);
    printf("Speedup (Escalar / AVX): %.2fx | Reducao de iteracoes do laco: 8x\n", speedup);

    liberar_vetor(a);
    liberar_vetor(b);
    liberar_vetor(res_esc);
    liberar_vetor(res_avx);
    return ok;
}

int main(void) {
    printf("====================================================================\n");
    printf("BENCHMARK AVX (256 bits - _mm256_sub_ps) - TP2 ARQUITETURA II\n");
    printf("====================================================================\n");

    // Ambos os tamanhos deixam tres elementos para o laco escalar residual.
    int ok_menor = executar_benchmark(1000003, 160, "Vetores menores");
    int ok_maior = executar_benchmark(8000003, 24, "Vetores maiores");
    return ok_menor && ok_maior ? EXIT_SUCCESS : EXIT_FAILURE;
}
