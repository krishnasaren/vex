// vex_rt/arena.c — Bump-pointer arena allocator

#include "vex_rt/arena.h"
#include "vex_rt/alloc.h"
#include "vex_rt/panic.h"
#include <string.h>

VexArena* vex_arena_create(uint64_t capacity) {
    if (capacity == 0) capacity = 65536;
    VexArena* a = (VexArena*)vex_alloc(sizeof(VexArena), _Alignof(VexArena));
    a->base     = (char*)vex_alloc(capacity, 16);
    a->cursor   = a->base;
    a->end      = a->base + capacity;
    a->capacity = capacity;
    return a;
}

void* vex_arena_alloc(VexArena* a, uint64_t size, uint64_t align) {
    if (align == 0) align = 1;
    uintptr_t cur   = (uintptr_t)a->cursor;
    uintptr_t rem   = cur % align;
    uintptr_t pad   = rem ? (align - rem) : 0;
    char*     ptr   = (char*)(cur + pad);
    if (ptr + size > a->end)
        vex_panic("vex_arena_alloc: arena exhausted", __FILE__, __LINE__);
    a->cursor = ptr + size;
    return ptr;
}

void vex_arena_reset(VexArena* a) { a->cursor = a->base; }

void vex_arena_destroy(VexArena* a) {
    if (!a) return;
    vex_free(a->base);
    vex_free(a);
}
