#ifndef EBA_ALLOCATOR_H
#define EBA_ALLOCATOR_H
#include <stddef.h>

#define EBA_ALIGN_UP(size, align) (((size) + (align) - 1) & ~((align) - 1))

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

    void *context;
}EbaAllocator;

extern const EbaAllocator eba_std_allocator;


#endif