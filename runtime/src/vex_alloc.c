/* vex_alloc.c — Memory allocation (wraps system allocator) */
#include "../include/vex_rt.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

void* vex_alloc(size_t size, size_t align) {
    if (size == 0) return NULL;
#if defined(__GLIBC__) || defined(__linux__)
    void* ptr = NULL;
    if (posix_memalign(&ptr, align < sizeof(void*) ? sizeof(void*) : align, size) != 0) {
        fprintf(stderr, "vex: out of memory (alloc %zu bytes)\n", size);
        abort();
    }
    return ptr;
#else
    (void)align;
    void* ptr = malloc(size);
    if (!ptr) { fprintf(stderr, "vex: out of memory\n"); abort(); }
    return ptr;
#endif
}

void* vex_alloc_zeroed(size_t size, size_t align) {
    void* ptr = vex_alloc(size, align);
    if (ptr) memset(ptr, 0, size);
    return ptr;
}

void* vex_realloc(void* ptr, size_t old_size, size_t new_size, size_t align) {
    (void)old_size; (void)align;
    if (!ptr) return vex_alloc(new_size, align);
    if (new_size == 0) { vex_free(ptr, old_size, align); return NULL; }
    void* np = realloc(ptr, new_size);
    if (!np) { fprintf(stderr, "vex: out of memory (realloc %zu)\n", new_size); abort(); }
    return np;
}

void vex_free(void* ptr, size_t size, size_t align) {
    (void)size; (void)align;
    free(ptr);
}
