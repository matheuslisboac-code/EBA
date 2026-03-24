#include <stddef.h>

typedef void *(*EbaAllocFn)(void *ctx, size_t size);
typedef void *(*EbaAlignAllocFn)(void *ctx, size_t size, size_t align);
typedef void *(*EbaReallocFn)(void *ctx, void *ptr, size_t old_size, size_t new_size);
typedef void *(*EbaAlignReallocFn)(void *ctx, void *ptr, size_t old_size, size_t new_size, size_t align);
typedef void (*EbaFreeFn)(void *ctx, void *ptr);

typedef struct{
    EbaAllocFn alloc;
    EbaAlignAllocFn aln_alloc;
    EbaReallocFn realloc;
    EbaAlignReallocFn aln_realloc;
    EbaFreeFn free;
}EbaAllocator;
