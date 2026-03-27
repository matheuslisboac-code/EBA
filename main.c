#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include "eba_hash.h"

// Quantas vezes repetir a operação para o relógio conseguir capturar o tempo
#define TEST_ITERATIONS 10000

// Função auxiliar para gerar nomes de chaves (ex: "entidade_42")
void generate_key(char *buffer, int i) {
    snprintf(buffer, 32, "entidade_%d", i);
}

int main() {
    EbaHash hash;
    clock_t start, end;
    double time_spent;
    char key[32];
    int value;

    printf("--- Iniciando Benchmark EbaHash ---\n\n");

    // Inicializa a tabela para strings de até 31 caracteres (32 bytes com o '\0')
    // Capacidade inicial: 128. Threshold será 96 (128 * 0.75).
    eba_hash_init_str(&hash, 128, sizeof(int), 31, NULL);

    // ==========================================
    // ETAPA 1: Inserir e remover sem crescer (Capacidade 128)
    // Vamos usar 90 itens para não atingir o threshold de 96.
    // ==========================================
    start = clock();

    for (int iter = 0; iter < TEST_ITERATIONS; iter++) {
        // Inserir 90 itens
        for (int i = 0; i < 90; i++) {
            generate_key(key, i);
            value = i * 10;
            eba_hash_put_str(&hash, key, &value);
        }
        // Remover 90 itens (Deixando tombstones que serão reaproveitados)
        for (int i = 0; i < 90; i++) {
            generate_key(key, i);
            eba_hash_remove_str(&hash, key);
        }
    }

    end = clock();
    time_spent = (double)(end - start) / CLOCKS_PER_SEC;
    printf("[Etapa 1] Inserir/Remover (90 itens, sem resize)\n");
    printf("-> Capacidade da tabela: %zu\n", hash.capacity);
    printf("-> Tempo total (%d loops): %f segundos\n\n", TEST_ITERATIONS, time_spent);


    // ==========================================
    // ETAPA 2: Limpar e forçar 2 crescimentos (128 -> 256 -> 512)
    // Para crescer duas vezes, precisamos passar do threshold de 256 (que é 192).
    // Vamos inserir 200 itens.
    // ==========================================
    start = clock();

    for (int iter = 0; iter < TEST_ITERATIONS; iter++) {
        // O tempo de limpar a tabela conta para a Etapa 2
        eba_hash_clear(&hash); 
        
        // Na primeira iteração do laço, ela vai crescer.
        // Nas seguintes, como não estamos resetando a capacidade, ela apenas insere rápido.
        for (int i = 0; i < 200; i++) {
            generate_key(key, i);
            value = i * 10;
            eba_hash_put_str(&hash, key, &value);
        }
    }

    end = clock();
    time_spent = (double)(end - start) / CLOCKS_PER_SEC;
    printf("[Etapa 2] Clear + Inserir Forcando Grow (200 itens)\n");
    printf("-> Capacidade final da tabela: %zu\n", hash.capacity);
    printf("-> Tempo total (%d loops): %f segundos\n\n", TEST_ITERATIONS, time_spent);


    // ==========================================
    // ETAPA 3: Limpar e repetir a Etapa 1, mas com a capacidade 512
    // ==========================================
    start = clock();

    for (int iter = 0; iter < TEST_ITERATIONS; iter++) {
        // O tempo de limpar a tabela conta para a Etapa 3
        eba_hash_clear(&hash);

        // Mesma quantidade da Etapa 1 (90 itens), mas num array muito maior
        for (int i = 0; i < 90; i++) {
            generate_key(key, i);
            value = i * 10;
            eba_hash_put_str(&hash, key, &value);
        }
        for (int i = 0; i < 90; i++) {
            generate_key(key, i);
            eba_hash_remove_str(&hash, key);
        }
    }

    end = clock();
    time_spent = (double)(end - start) / CLOCKS_PER_SEC;
    printf("[Etapa 3] Clear + Inserir/Remover (90 itens na tabela grande)\n");
    printf("-> Capacidade da tabela: %zu\n", hash.capacity);
    printf("-> Tempo total (%d loops): %f segundos\n\n", TEST_ITERATIONS, time_spent);

    // Limpeza final da memória
    eba_hash_free(&hash);
    printf("--- Fim do Benchmark ---\n");

    return 0;
}