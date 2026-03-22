#pragma once
#ifdef __cplusplus
extern "C" {
#endif
#include <stdint.h>

typedef struct VexArena {
    char*    base;
    char*    cursor;
    char*    end;
    uint64_t capacity;
} VexArena;

VexArena* vex_arena_create(uint64_t capacity);
void*     vex_arena_alloc(VexArena* a, uint64_t size, uint64_t align);
void      vex_arena_reset(VexArena* a);
void      vex_arena_destroy(VexArena* a);

#ifdef __cplusplus
}
#endif
