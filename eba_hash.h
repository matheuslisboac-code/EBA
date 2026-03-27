#ifndef EBA_HASH_H
#define EBA_HASH_H

#include "eba_allocator.h"
#include "eba_returns.h"
#include <stddef.h>
#include <stdint.h>

// --- Definições Padrão ---
#define EBA_HASH_GROW_FACTOR         2
#define EBA_HASH_MIN_CAPACITY        8
#define EBA_HASH_DEFAULT_LOAD_FACTOR 0.75f
#define EBA_HASH_DEFAULT_ALIGN       8

// --- Assinaturas de Funções de Hash e Comparação ---
typedef size_t (*EbaHashFn)(void *key);
typedef int (*EbaHashKeyCmpFn)(void *key1, void *key2);

typedef enum{
    EBA_HASH_ITEM_EMPTY,
    EBA_HASH_ITEM_FILL,
    EBA_HASH_ITEM_REMOVED,

}EbaHashItemState; 

// --- Estrutura Principal ---
typedef struct {
    const EbaAllocator *allocator;

    size_t capacity;
    float load_factor;
    size_t threshold;
    size_t size;
    size_t used_slots;

    size_t item_size;
    size_t item_align;
    
    size_t key_size;
    size_t key_align;

    EbaHashFn hash_fn;
    EbaHashKeyCmpFn key_cmp;

    uint8_t *states;
    uint8_t *keys;
    uint8_t *items;
} EbaHash;

// --- API Principal (Genérica) ---

EbaResult eba_hash_init_pro(
    EbaHash *hash, size_t capacity, float load_factor, 
    size_t item_size, size_t item_align, 
    size_t key_size, size_t key_align,
    EbaHashFn hash_fn, EbaHashKeyCmpFn key_cmp, 
    const EbaAllocator *allocator
);

EbaResult eba_hash_init(
    EbaHash *hash, size_t capacity, 
    size_t item_size, size_t key_size,
    EbaHashFn hash_fn, EbaHashKeyCmpFn key_cmp, 
    const EbaAllocator *allocator
);

EbaResult eba_hash_put(EbaHash *hash, void *key, void *item);
void *eba_hash_get(EbaHash *hash, void *key);
void eba_hash_remove(EbaHash *hash, void *key);

void eba_hash_free(EbaHash *hash);
void eba_hash_clear(EbaHash *hash);

// Funções utilitárias para iteração manual
void *eba_hash_get_index_key(EbaHash *hash, size_t index);
void *eba_hash_get_index_item(EbaHash *hash, size_t index);

// --- API Específica para Strings (SSO) ---

int eba_hash_str_cmp(void *key1, void *key2);
size_t eba_hash_str_fnv1a(void *key);

EbaResult eba_hash_init_str(
    EbaHash *hash, size_t capacity, 
    size_t item_size, size_t max_str_length, 
    const EbaAllocator *allocator
);

EbaResult eba_hash_put_str(EbaHash *hash, const char *key_str, void *item);
void *eba_hash_get_str(EbaHash *hash, const char *key_str);
void eba_hash_remove_str(EbaHash *hash, const char *key_str);

int eba_hash_int_cmp(void *key1, void *key2);
size_t eba_hash_int_hash(void *key);

EbaResult eba_hash_init_int(
    EbaHash *hash, size_t capacity, 
    size_t item_size, const EbaAllocator *allocator
);



#endif // EBA_HASH_H