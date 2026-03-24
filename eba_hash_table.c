#include "eba_allocator.h"
#include <stddef.h>
#include <stdint.h>
#include <string.h>

typedef enum{
    EBA_HASH_ITEM_EMPTY,
    EBA_HASH_ITEM_FILL,
    EBA_HASH_ITEM_REMOVED,
}EbaHashItemState;

typedef size_t (*EbaHashFn)(void *key);
typedef int (*EbaHashKeyCmpFn)(void *key1, void *key2);

typedef struct{
    EbaAllocator allocator;

    size_t capacity;
    size_t size;

    size_t type_size;
    size_t type_align;
    
    size_t key_size;
    size_t key_align;

    EbaHashFn hash_fn;
    EbaHashKeyCmpFn key_cmp;

    uint8_t *states;
    uint8_t *keys;
    uint8_t *items;
}EbaHash;


void eba_hash_table_grow(EbaHash *hash, size_t new_capacity){
    
}

void *eba_hash_put_item(EbaHash *hash, void *key, void *item){

}