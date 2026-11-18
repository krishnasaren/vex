/* vex_map.c — HashMap runtime (open addressing, FNV hash by default) */
#include "../include/vex_rt.h"
#include <string.h>

#define MAP_LOAD_FACTOR 70  /* 70% load → grow */
#define MAP_INIT_CAP    16

typedef struct { void* key; void* val; uint64_t hash; int8_t state; } Entry;
#define ENTRY_EMPTY  0
#define ENTRY_FULL   1
#define ENTRY_DEAD   2

struct VexHashMap_ {
    Entry*   entries;
    size_t   cap;
    size_t   len;
    size_t   key_size;
    size_t   val_size;
    uint64_t (*hasher)(const void*);
    bool     (*eq)(const void*, const void*);
};

static uint64_t str_hash(const void* k) {
    VexStr s = *(const VexStr*)k;
    uint64_t h = 14695981039346656037ULL;
    for (size_t i = 0; i < s.len; i++)
        h = (h ^ (uint8_t)s.ptr[i]) * 1099511628211ULL;
    return h;
}

static bool str_eq(const void* a, const void* b) {
    return vex_str_eq(*(const VexStr*)a, *(const VexStr*)b);
}

VexHashMap* vex_map_new(size_t ks, size_t vs,
                         uint64_t(*h)(const void*), bool(*e)(const void*,const void*)) {
    VexHashMap* m = (VexHashMap*)vex_alloc_zeroed(sizeof(*m), 8);
    m->key_size = ks; m->val_size = vs;
    m->hasher = h ? h : str_hash;
    m->eq     = e ? e : str_eq;
    m->cap    = MAP_INIT_CAP;
    m->entries= (Entry*)vex_alloc_zeroed(sizeof(Entry)*m->cap, 8);
    for (size_t i=0;i<m->cap;i++) {
        m->entries[i].key = vex_alloc(ks,8);
        m->entries[i].val = vex_alloc(vs,8);
    }
    return m;
}

static Entry* find_slot(VexHashMap* m, const void* key, uint64_t h) {
    size_t i = h % m->cap;
    for (size_t d=0; d<m->cap; d++, i=(i+1)%m->cap) {
        if (m->entries[i].state == ENTRY_EMPTY) return &m->entries[i];
        if (m->entries[i].state == ENTRY_FULL  &&
            m->entries[i].hash == h &&
            m->eq(m->entries[i].key, key))
            return &m->entries[i];
    }
    return NULL;
}

void vex_map_insert(VexHashMap* m, const void* key, const void* val) {
    if (m->len * 100 / m->cap >= MAP_LOAD_FACTOR) {
        /* Grow */
        size_t old_cap = m->cap;
        Entry* old     = m->entries;
        m->cap  *= 2;
        m->len   = 0;
        m->entries = (Entry*)vex_alloc_zeroed(sizeof(Entry)*m->cap, 8);
        for (size_t i=0;i<m->cap;i++){
            m->entries[i].key=vex_alloc(m->key_size,8);
            m->entries[i].val=vex_alloc(m->val_size,8);
        }
        for (size_t i=0; i<old_cap; i++)
            if (old[i].state==ENTRY_FULL)
                vex_map_insert(m, old[i].key, old[i].val);
        for (size_t i=0;i<old_cap;i++){
            vex_free(old[i].key,m->key_size,8);
            vex_free(old[i].val,m->val_size,8);
        }
        vex_free(old, sizeof(Entry)*old_cap, 8);
    }
    uint64_t h = m->hasher(key);
    Entry* e = find_slot(m, key, h);
    if (!e) return;
    if (e->state != ENTRY_FULL) m->len++;
    memcpy(e->key, key, m->key_size);
    memcpy(e->val, val, m->val_size);
    e->hash = h; e->state = ENTRY_FULL;
}

void* vex_map_get(const VexHashMap* m, const void* key) {
    uint64_t h = m->hasher(key);
    size_t i = h % m->cap;
    for (size_t d=0; d<m->cap; d++, i=(i+1)%m->cap) {
        if (m->entries[i].state==ENTRY_EMPTY) return NULL;
        if (m->entries[i].state==ENTRY_FULL &&
            m->entries[i].hash==h && m->eq(m->entries[i].key,key))
            return m->entries[i].val;
    }
    return NULL;
}

bool vex_map_remove(VexHashMap* m, const void* key) {
    uint64_t h = m->hasher(key);
    size_t i = h % m->cap;
    for (size_t d=0; d<m->cap; d++, i=(i+1)%m->cap) {
        if (m->entries[i].state==ENTRY_EMPTY) return false;
        if (m->entries[i].state==ENTRY_FULL &&
            m->entries[i].hash==h && m->eq(m->entries[i].key,key)) {
            m->entries[i].state = ENTRY_DEAD;
            m->len--;
            return true;
        }
    }
    return false;
}

size_t vex_map_len(const VexHashMap* m) { return m->len; }

void vex_map_free(VexHashMap* m) {
    for (size_t i=0;i<m->cap;i++){
        vex_free(m->entries[i].key,m->key_size,8);
        vex_free(m->entries[i].val,m->val_size,8);
    }
    vex_free(m->entries,sizeof(Entry)*m->cap,8);
    vex_free(m,sizeof(*m),8);
}
