/* vex_array.c — Dynamic array (Vec<T>) runtime */
#include "../include/vex_rt.h"
#include <string.h>
#include <stdlib.h>

VexVec vex_vec_new(size_t elem_size) {
    return (VexVec){ .ptr=NULL, .len=0, .cap=0, .elem_size=elem_size };
}

VexVec vex_vec_with_capacity(size_t elem_size, size_t cap) {
    void* ptr = vex_alloc(elem_size * cap, 8);
    return (VexVec){ .ptr=ptr, .len=0, .cap=cap, .elem_size=elem_size };
}

static void vec_grow(VexVec* v, size_t min_cap) {
    size_t new_cap = v->cap == 0 ? 8 : v->cap * 2;
    while (new_cap < min_cap) new_cap *= 2;
    v->ptr = vex_realloc(v->ptr, v->cap * v->elem_size, new_cap * v->elem_size, 8);
    v->cap = new_cap;
}

void vex_vec_push(VexVec* v, const void* elem) {
    if (v->len == v->cap) vec_grow(v, v->len + 1);
    memcpy((char*)v->ptr + v->len * v->elem_size, elem, v->elem_size);
    v->len++;
}

void* vex_vec_get(const VexVec* v, size_t idx) {
    if (idx >= v->len) vex_panic_oob(idx, v->len, __FILE__, __LINE__);
    return (char*)v->ptr + idx * v->elem_size;
}

void vex_vec_set(VexVec* v, size_t idx, const void* elem) {
    if (idx >= v->len) vex_panic_oob(idx, v->len, __FILE__, __LINE__);
    memcpy((char*)v->ptr + idx * v->elem_size, elem, v->elem_size);
}

void vex_vec_pop(VexVec* v, void* out) {
    if (v->len == 0) vex_panic("pop on empty vec", 18, __FILE__, __LINE__);
    v->len--;
    if (out) memcpy(out, (char*)v->ptr + v->len * v->elem_size, v->elem_size);
}

void vex_vec_insert(VexVec* v, size_t idx, const void* elem) {
    if (idx > v->len) idx = v->len;
    if (v->len == v->cap) vec_grow(v, v->len + 1);
    memmove((char*)v->ptr + (idx+1)*v->elem_size,
            (char*)v->ptr + idx*v->elem_size,
            (v->len - idx) * v->elem_size);
    memcpy((char*)v->ptr + idx*v->elem_size, elem, v->elem_size);
    v->len++;
}

void vex_vec_remove(VexVec* v, size_t idx) {
    if (idx >= v->len) vex_panic_oob(idx, v->len, __FILE__, __LINE__);
    memmove((char*)v->ptr + idx*v->elem_size,
            (char*)v->ptr + (idx+1)*v->elem_size,
            (v->len - idx - 1) * v->elem_size);
    v->len--;
}

void vex_vec_clear(VexVec* v) { v->len = 0; }

void vex_vec_free(VexVec* v) {
    vex_free(v->ptr, v->cap * v->elem_size, 8);
    v->ptr=NULL; v->len=0; v->cap=0;
}

VexVec vex_vec_clone(const VexVec* v) {
    VexVec r = vex_vec_with_capacity(v->elem_size, v->len);
    memcpy(r.ptr, v->ptr, v->len * v->elem_size);
    r.len = v->len;
    return r;
}
