#pragma once
// vex_rt/alloc.h — VEX runtime memory allocation API

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>

// Allocate 'size' bytes with alignment 'align' (must be power of 2).
// Aborts on out-of-memory.
void* vex_alloc(uint64_t size, uint64_t align);

// Free a pointer previously returned by vex_alloc.
void vex_free(void* ptr);

// Reallocate memory. align must match original allocation.
void* vex_realloc(void* ptr, uint64_t new_size, uint64_t align);

// Zero-fill 'size' bytes at 'ptr'.
void vex_zero(void* ptr, uint64_t size);

// Copy 'size' bytes from src to dst (no overlap).
void vex_copy(void* dst, const void* src, uint64_t size);

// Copy 'size' bytes allowing overlap.
void vex_move_mem(void* dst, const void* src, uint64_t size);

#ifdef __cplusplus
}
#endif
