#include "eba_allocator.h"
#include <stdlib.h>
#include <string.h> 


static void *std_alloc(void *ctx, size_t size) {
    (void)ctx; // Suprime o warning de variável não utilizada
    return malloc(size);
}

static void *std_aln_alloc(void *ctx, size_t size, size_t align) {
    (void)ctx;
    return aligned_alloc(align, size);
}

static void *std_realloc(void *ctx, void *ptr, size_t old_size, size_t new_size) {
    (void)ctx;
    (void)old_size; 
    return realloc(ptr, new_size);
}

static void *std_aln_realloc(void *ctx, void *ptr, size_t old_size, size_t new_size, size_t align) {
    (void)ctx;

    if (new_size == 0) {
        free(ptr);
        return NULL;
    }

    if (ptr == NULL) {
        return aligned_alloc(align, new_size);
    }


    void *new_ptr = aligned_alloc(align, new_size);
    if (new_ptr == NULL) {
        return NULL;
    }

    size_t copy_size = (old_size < new_size) ? old_size : new_size;
    memcpy(new_ptr, ptr, copy_size);

    free(ptr);

    return new_ptr;
}

static void std_free(void *ctx, void *ptr) {
    (void)ctx;
    free(ptr);
}


const EbaAllocator eba_std_allocator = {
    .alloc       = std_alloc,
    .aln_alloc   = std_aln_alloc,
    .realloc     = std_realloc,
    .aln_realloc = std_aln_realloc,
    .free        = std_free,
    .context     = NULL
};