/* vex_string.c — String runtime functions */
#include "../include/vex_rt.h"
#include <string.h>
#include <stdlib.h>

VexStr vex_str_from_cstr(const char* s) {
    return (VexStr){ .ptr = s, .len = s ? strlen(s) : 0 };
}

VexStr vex_str_slice(VexStr s, size_t start, size_t end) {
    if (start > s.len) start = s.len;
    if (end   > s.len) end   = s.len;
    if (end   < start) end   = start;
    return (VexStr){ .ptr = s.ptr + start, .len = end - start };
}

bool vex_str_eq(VexStr a, VexStr b) {
    return a.len == b.len && memcmp(a.ptr, b.ptr, a.len) == 0;
}

int vex_str_cmp(VexStr a, VexStr b) {
    size_t n = a.len < b.len ? a.len : b.len;
    int r = memcmp(a.ptr, b.ptr, n);
    if (r != 0) return r;
    return (a.len > b.len) - (a.len < b.len);
}

bool vex_str_contains(VexStr haystack, VexStr needle) {
    if (needle.len == 0) return true;
    if (needle.len > haystack.len) return false;
    for (size_t i = 0; i + needle.len <= haystack.len; i++) {
        if (memcmp(haystack.ptr + i, needle.ptr, needle.len) == 0)
            return true;
    }
    return false;
}

bool vex_str_starts_with(VexStr s, VexStr p) {
    return p.len <= s.len && memcmp(s.ptr, p.ptr, p.len) == 0;
}

bool vex_str_ends_with(VexStr s, VexStr p) {
    return p.len <= s.len &&
           memcmp(s.ptr + s.len - p.len, p.ptr, p.len) == 0;
}

VexString vex_string_new(void) {
    return (VexString){ .ptr = NULL, .len = 0, .cap = 0 };
}

VexString vex_string_with_capacity(size_t cap) {
    char* p = (char*)vex_alloc(cap + 1, 1);
    return (VexString){ .ptr = p, .len = 0, .cap = cap };
}

static void string_reserve(VexString* s, size_t additional) {
    if (s->len + additional <= s->cap) return;
    size_t new_cap = (s->cap == 0) ? 16 : s->cap * 2;
    while (new_cap < s->len + additional) new_cap *= 2;
    s->ptr = (char*)vex_realloc(s->ptr, s->cap, new_cap + 1, 1);
    s->cap = new_cap;
}

void vex_string_push(VexString* s, VexStr slice) {
    string_reserve(s, slice.len);
    memcpy(s->ptr + s->len, slice.ptr, slice.len);
    s->len += slice.len;
    s->ptr[s->len] = '\0';
}

void vex_string_push_char(VexString* s, uint32_t cp) {
    char buf[4];
    size_t n = 0;
    if (cp < 0x80) { buf[n++] = (char)cp; }
    else if (cp < 0x800)   { buf[n++] = (char)(0xC0|(cp>>6));  buf[n++]=(char)(0x80|(cp&0x3F)); }
    else if (cp < 0x10000) { buf[n++]=(char)(0xE0|(cp>>12)); buf[n++]=(char)(0x80|((cp>>6)&0x3F)); buf[n++]=(char)(0x80|(cp&0x3F)); }
    else { buf[n++]=(char)(0xF0|(cp>>18)); buf[n++]=(char)(0x80|((cp>>12)&0x3F)); buf[n++]=(char)(0x80|((cp>>6)&0x3F)); buf[n++]=(char)(0x80|(cp&0x3F)); }
    vex_string_push(s, (VexStr){.ptr=buf,.len=n});
}

VexStr vex_string_as_str(const VexString* s) {
    return (VexStr){ .ptr = s->ptr ? s->ptr : "", .len = s->len };
}

void vex_string_free(VexString* s) {
    vex_free(s->ptr, s->cap, 1);
    s->ptr = NULL; s->len = 0; s->cap = 0;
}

VexString vex_string_concat(VexStr a, VexStr b) {
    VexString s = vex_string_with_capacity(a.len + b.len);
    vex_string_push(&s, a);
    vex_string_push(&s, b);
    return s;
}

VexString vex_string_repeat(VexStr s, size_t n) {
    VexString r = vex_string_with_capacity(s.len * n);
    for (size_t i = 0; i < n; i++) vex_string_push(&r, s);
    return r;
}
