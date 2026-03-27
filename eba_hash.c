#include "eba_hash.h"
#include <string.h>


EbaResult eba_hash_init_pro(
    EbaHash *hash, size_t capacity, float load_factor, 
    size_t item_size, size_t item_align, 
    size_t key_size, size_t key_align,
    EbaHashFn hash_fn, EbaHashKeyCmpFn key_cmp, 
    const EbaAllocator *allocator
) {
    // 1. Validação de segurança básica
    if (!hash || !hash_fn || !key_cmp || item_size == 0 || key_size == 0) {
        return EBA_ERROR_INVALID_PARAM;
    }

    // 2. Configuração do Alocador
    hash->allocator = allocator ? allocator : &eba_std_allocator;

    // 3. Aplicação dos valores padrão via Macros
    if (capacity < EBA_HASH_MIN_CAPACITY) capacity = EBA_HASH_MIN_CAPACITY;
    if (load_factor <= 0.0f || load_factor >= 1.0f) load_factor = EBA_HASH_DEFAULT_LOAD_FACTOR;
    if (item_align == 0) item_align = EBA_HASH_DEFAULT_ALIGN;
    if (key_align == 0) key_align = EBA_HASH_DEFAULT_ALIGN;

    // 4. Preenchimento do estado inicial
    hash->capacity = capacity;
    hash->size = 0;
    hash->used_slots = 0;
    hash->load_factor = load_factor;
    hash->threshold = (size_t)(capacity * load_factor);
    
    hash->item_size = item_size;
    hash->item_align = item_align;
    hash->key_size = key_size;
    hash->key_align = key_align;
    
    hash->hash_fn = hash_fn;
    hash->key_cmp = key_cmp;

    void *ctx = hash->allocator->context;

    // 5. Alocações
    hash->states = (uint8_t *)hash->allocator->alloc(ctx, capacity);
    if (!hash->states) return EBA_ERROR_OUT_OF_MEMORY;

    hash->keys = (uint8_t *)hash->allocator->aln_alloc(ctx, capacity * key_size, key_align);
    if (!hash->keys) {
        hash->allocator->free(ctx, hash->states);
        return EBA_ERROR_OUT_OF_MEMORY;
    }

    hash->items = (uint8_t *)hash->allocator->aln_alloc(ctx, capacity * item_size, item_align);
    if (!hash->items) {
        hash->allocator->free(ctx, hash->states);
        hash->allocator->free(ctx, hash->keys);
        return EBA_ERROR_OUT_OF_MEMORY;
    }

    memset(hash->states, 0, capacity);

    return EBA_OK;
}

EbaResult eba_hash_init(
    EbaHash *hash, size_t capacity, 
    size_t item_size, size_t key_size,
    EbaHashFn hash_fn, EbaHashKeyCmpFn key_cmp, 
    const EbaAllocator *allocator
) {
    return eba_hash_init_pro(
        hash, 
        capacity, 
        EBA_HASH_DEFAULT_LOAD_FACTOR, 
        item_size, EBA_HASH_DEFAULT_ALIGN, 
        key_size, EBA_HASH_DEFAULT_ALIGN, 
        hash_fn, key_cmp, 
        allocator
    );
}


static void _eba_hash_put(EbaHash *hash, void *key, void *item){
    size_t index = hash->hash_fn(key) % hash->capacity;
    while (hash->states[index] == EBA_HASH_ITEM_FILL) {
        index = index + 1 >= hash->capacity ? 0 : index + 1;
    }
    memcpy((void *)(hash->keys + (index * hash->key_size)), key, hash->key_size);
    memcpy((void *)(hash->items + (index * hash->item_size)), item, hash->item_size);
    hash->states[index] = EBA_HASH_ITEM_FILL;
    hash->size++;
    hash->used_slots++;
}

static EbaResult _eba_hash_resize(EbaHash *hash, size_t new_capacity){
    
    uint8_t *old_states = hash->states;
    uint8_t *old_keys = hash->keys;
    uint8_t *old_items = hash->items;
    size_t old_capacity = hash->capacity;

    hash->states = hash->allocator->alloc(hash->allocator->context, new_capacity);
    if(!(hash->states)){
        hash->states = old_states;
        return EBA_ERROR_OUT_OF_MEMORY;
    }
    hash->keys = hash->allocator->aln_alloc(hash->allocator->context, new_capacity * hash->key_size, hash->key_align);
    if(!(hash->keys)){
        hash->allocator->free(hash->allocator->context, hash->states);
        hash->states = old_states;
        hash->keys = old_keys;
        return EBA_ERROR_OUT_OF_MEMORY;
    }
    hash->items = hash->allocator->aln_alloc(hash->allocator->context, new_capacity * hash->item_size, hash->item_align);
    if(!(hash->items)){
        hash->allocator->free(hash->allocator->context, hash->states);
        hash->allocator->free(hash->allocator->context, hash->keys);        
        hash->states = old_states;
        hash->keys = old_keys;
        hash->items = old_items;
        return EBA_ERROR_OUT_OF_MEMORY;
    }
    hash->capacity = new_capacity;
    hash->threshold = (size_t)(hash->capacity * hash->load_factor);    
    hash->size = 0;
    memset((void *)(hash->states), 0, new_capacity);

    for(size_t index = 0; index < old_capacity; index++){
        if(old_states[index] == EBA_HASH_ITEM_FILL){
            _eba_hash_put(hash, (void *)(old_keys + (index * hash->key_size)), (void *)(old_items + index * hash->item_size));
        }
    }
    hash->allocator->free(hash->allocator->context, old_items);
    hash->allocator->free(hash->allocator->context, old_keys);
    hash->allocator->free(hash->allocator->context, old_states); 
    return EBA_OK;
}

void *eba_hash_get_index_key(EbaHash *hash, size_t index){
    if(index >= hash->capacity) return NULL;
    return (void *)(hash->keys + (hash->key_size * index));
}

void *eba_hash_get_index_item(EbaHash *hash, size_t index){
    if(index >= hash->capacity) return NULL;
    return (void *)(hash->items + (hash->item_size * index));
}

EbaResult eba_hash_put(EbaHash *hash, void *key, void *item){
    if(!hash || !key || !item){
        return EBA_ERROR_INVALID_PARAM;
    }
    if (hash->used_slots + 1 >= hash->threshold) { 
        EbaResult result = _eba_hash_resize(hash, hash->capacity * EBA_HASH_GROW_FACTOR);
        if (result != EBA_OK) return result;
    }

    size_t index = hash->hash_fn(key) % hash->capacity;
    uint8_t find_removed = 0;
    size_t first_removed = 0;
    uint8_t is_new_key = 1;
    uint8_t corr_state = hash->states[index];
    while (corr_state != EBA_HASH_ITEM_EMPTY) {
        if(corr_state == EBA_HASH_ITEM_FILL && hash->key_cmp(key, eba_hash_get_index_key(hash, index))){
            find_removed = 0;
            is_new_key = 0;
            break;
        }else if (corr_state == EBA_HASH_ITEM_REMOVED && !find_removed) {
            find_removed = 1;
            first_removed = index;
        }
        index = (index + 1) >= hash->capacity ? 0 : index + 1;
        corr_state = hash->states[index];
    }
    if(find_removed) index = first_removed;

    hash->states[index] = EBA_HASH_ITEM_FILL;
    memcpy((void *)(hash->keys + (hash->key_size * index)), key, hash->key_size);
    memcpy((void *)(hash->items + (hash->item_size * index)), item, hash->item_size);
    if (is_new_key) {
        hash->size++;
        if (!find_removed) {
            hash->used_slots++;
        }
    }
    

    return EBA_OK;
}

void *eba_hash_get(EbaHash *hash, void *key){
    if(hash->capacity == 0) return NULL;
    size_t index = hash->hash_fn(key) % hash->capacity;
    uint8_t corr_state = hash->states[index];
    while (corr_state != EBA_HASH_ITEM_EMPTY) {
        if(corr_state == EBA_HASH_ITEM_FILL && hash->key_cmp(key, eba_hash_get_index_key(hash, index)))
            return eba_hash_get_index_item(hash, index);

        index = (index + 1) % hash->capacity;
        corr_state = hash->states[index];
    }

    return NULL;
}

void eba_hash_remove(EbaHash *hash, void *key){
    if(hash->capacity == 0) return;
    size_t index = hash->hash_fn(key) % hash->capacity;
    uint8_t corr_state = hash->states[index];
    while (corr_state != EBA_HASH_ITEM_EMPTY) {
        if(corr_state == EBA_HASH_ITEM_FILL && hash->key_cmp(key, eba_hash_get_index_key(hash, index))){
            hash->states[index] = EBA_HASH_ITEM_REMOVED;
            hash->size--;
            break;
        }

        index = (index + 1) % hash->capacity;
        corr_state = hash->states[index];
    }
}

void eba_hash_free(EbaHash *hash){
    hash->allocator->free(hash->allocator->context, hash->states);
    hash->allocator->free(hash->allocator->context, hash->keys);
    hash->allocator->free(hash->allocator->context, hash->items);
    hash->capacity = 0;
    hash->size = 0;
}

void eba_hash_clear(EbaHash *hash){
    memset(hash->states, 0, hash->capacity);
    hash->size = 0;
    hash->used_slots = 0;
}


int eba_hash_str_cmp(void *key1, void *key2) {
    return strcmp((const char *)key1, (const char *)key2) == 0;
}

size_t eba_hash_str_fnv1a(void *key) {
    const char *str = (const char *)key;
    size_t hash = 14695981039346656037ULL;
    
    while (*str) {
        hash ^= (size_t)(unsigned char)(*str);
        hash *= 1099511628211ULL; // FNV prime
        str++;
    }
    return hash;
}

EbaResult eba_hash_init_str(
    EbaHash *hash, size_t capacity, 
    size_t item_size, size_t max_str_length, 
    const EbaAllocator *allocator
) {
    size_t key_size = max_str_length + 1;
    
    size_t key_align = 1; 

    return eba_hash_init_pro(
        hash, capacity, EBA_HASH_DEFAULT_LOAD_FACTOR, 
        item_size, EBA_HASH_DEFAULT_ALIGN, 
        key_size, key_align, 
        eba_hash_str_fnv1a, eba_hash_str_cmp, 
        allocator
    );
}

EbaResult eba_hash_put_str(EbaHash *hash, const char *key_str, void *item) {
    if (!hash || !key_str || !item) return EBA_ERROR_INVALID_PARAM;

    char safe_key[hash->key_size];
    
    strncpy(safe_key, key_str, hash->key_size);
    
    safe_key[hash->key_size - 1] = '\0'; 

    return eba_hash_put(hash, safe_key, item);
}

void *eba_hash_get_str(EbaHash *hash, const char *key_str) {
    if (!hash || !key_str) return NULL;

    return eba_hash_get(hash, (void *)key_str);
}

void eba_hash_remove_str(EbaHash *hash, const char *key_str) {
    if (!hash || !key_str) return;
    eba_hash_remove(hash, (void *)key_str);
}