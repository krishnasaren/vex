// vex_rt/arc.c — Atomic Reference Counting (~T shared pointer runtime)

#include "vex_rt/arc.h"
#include "vex_rt/alloc.h"
#include "vex_rt/panic.h"
#include <stdint.h>
#include <stddef.h>

#if defined(__GNUC__) || defined(__clang__)
#  define VEX_ATOMIC_LOAD(p)         __atomic_load_n((p), __ATOMIC_SEQ_CST)
#  define VEX_ATOMIC_ADD(p,v)        __atomic_fetch_add((p),(v),__ATOMIC_SEQ_CST)
#  define VEX_ATOMIC_SUB(p,v)        __atomic_fetch_sub((p),(v),__ATOMIC_SEQ_CST)
#  define VEX_ATOMIC_CAS(p,e,d)      __atomic_compare_exchange_n((p),(e),(d),1,__ATOMIC_ACQ_REL,__ATOMIC_ACQUIRE)
#elif defined(_MSC_VER)
#  include <intrin.h>
#  define VEX_ATOMIC_LOAD(p)         (*(volatile uint64_t*)(p))
#  define VEX_ATOMIC_ADD(p,v)        ((uint64_t)_InterlockedExchangeAdd64((__int64*)(p),(int64_t)(v)))
#  define VEX_ATOMIC_SUB(p,v)        ((uint64_t)_InterlockedExchangeAdd64((__int64*)(p),-(int64_t)(v)))
static inline int vex_cas(volatile uint64_t* p, uint64_t* e, uint64_t d) {
    uint64_t old = (uint64_t)_InterlockedCompareExchange64((__int64*)p,(int64_t)d,(int64_t)*e);
    if (old == *e) return 1; *e = old; return 0;
}
#  define VEX_ATOMIC_CAS(p,e,d)      vex_cas((p),(e),(d))
#else
#  error "Unsupported compiler"
#endif

VexArcHeader* vex_arc_alloc(uint64_t data_size, uint64_t data_align, VexDropFn drop_fn) {
    uint64_t header_size = sizeof(VexArcHeader);
    uint64_t padding     = (data_align > 0) ? (data_align - (header_size % data_align)) % data_align : 0;
    uint64_t total       = header_size + padding + data_size;
    uint64_t align       = (data_align > _Alignof(VexArcHeader)) ? data_align : _Alignof(VexArcHeader);
    VexArcHeader* h      = (VexArcHeader*)vex_alloc(total, align);
    h->strong_count = 1;
    h->weak_count   = 1;
    h->data_offset  = (uint32_t)(header_size + padding);
    h->drop_fn      = drop_fn;
    return h;
}

void* vex_arc_data(VexArcHeader* h) { return (char*)h + h->data_offset; }

VexArcHeader* vex_arc_clone(VexArcHeader* h) {
    if (!h) vex_panic("vex_arc_clone: null", __FILE__, __LINE__);
    uint64_t prev = VEX_ATOMIC_ADD(&h->strong_count, 1);
    if (prev == 0) vex_panic("vex_arc_clone: dead arc", __FILE__, __LINE__);
    return h;
}

void vex_arc_drop(VexArcHeader* h) {
    if (!h) return;
    if (VEX_ATOMIC_SUB(&h->strong_count, 1) == 1) {
        if (h->drop_fn) h->drop_fn(vex_arc_data(h));
        vex_arc_weak_drop(h);
    }
}

VexArcHeader* vex_arc_weak_clone(VexArcHeader* h) {
    if (!h) vex_panic("vex_arc_weak_clone: null", __FILE__, __LINE__);
    VEX_ATOMIC_ADD(&h->weak_count, 1);
    return h;
}

void vex_arc_weak_drop(VexArcHeader* h) {
    if (!h) return;
    if (VEX_ATOMIC_SUB(&h->weak_count, 1) == 1) vex_free(h);
}

VexArcHeader* vex_arc_weak_upgrade(VexArcHeader* h) {
    if (!h) return NULL;
    uint64_t s = VEX_ATOMIC_LOAD(&h->strong_count);
    while (s != 0) {
        if (VEX_ATOMIC_CAS(&h->strong_count, &s, s + 1)) return h;
    }
    return NULL;
}
