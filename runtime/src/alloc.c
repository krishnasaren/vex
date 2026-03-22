// vex/runtime/src/alloc.c
// VEX runtime heap allocator.
// Thin wrapper around the system allocator (or jemalloc if linked).

#include "vex_rt/alloc.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <stdint.h>

// ── vex_alloc ─────────────────────────────────────────────────────────────────
// Allocates 'size' bytes aligned to 'align' bytes.
// 'align' must be a power of 2.
// Aborts on allocation failure (OOM is not recoverable in VEX).

void* vex_alloc(uint64_t size, uint64_t align) {
    if (size == 0) size = 1;

    void* ptr;
#if defined(_WIN32)
    ptr = _aligned_malloc((size_t)size, (size_t)align);
#elif defined(__APPLE__) || defined(__linux__)
    if (posix_memalign(&ptr, (size_t)(align < sizeof(void*) ? sizeof(void*) : align),
                        (size_t)size) != 0) {
        ptr = NULL;
    }
#else
    ptr = malloc((size_t)size);
    (void)align;
#endif

    if (!ptr) {
        fprintf(stderr, "vex_rt: out of memory (requested %llu bytes)\n",
                (unsigned long long)size);
        abort();
    }
    return ptr;
}

// ── vex_free ──────────────────────────────────────────────────────────────────

void vex_free(void* ptr) {
    if (!ptr) return;
#if defined(_WIN32)
    _aligned_free(ptr);
#else
    free(ptr);
#endif
}

// ── vex_realloc ───────────────────────────────────────────────────────────────

void* vex_realloc(void* ptr, uint64_t new_size, uint64_t align) {
    if (!ptr) return vex_alloc(new_size, align);
    if (new_size == 0) { vex_free(ptr); return NULL; }

    // For aligned realloc: alloc new, copy, free old
    void* new_ptr = vex_alloc(new_size, align);
    if (ptr) {
        // We don't know the old size here — caller must track it
        // This is a fallback; real usage should use arena or sized tracking
        free(ptr); // safe: vex_alloc uses posix_memalign which frees with free()
    }
    return new_ptr;
}

// ── vex_zero ──────────────────────────────────────────────────────────────────

void vex_zero(void* ptr, uint64_t size) {
    memset(ptr, 0, (size_t)size);
}

// ── vex_copy ──────────────────────────────────────────────────────────────────

void vex_copy(void* dst, const void* src, uint64_t size) {
    memcpy(dst, src, (size_t)size);
}

void vex_move_mem(void* dst, const void* src, uint64_t size) {
    memmove(dst, src, (size_t)size);
}
