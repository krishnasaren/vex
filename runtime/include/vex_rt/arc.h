#pragma once
// vex_rt/arc.h — ARC (~T) runtime header
#ifdef __cplusplus
extern "C" {
#endif
#include <stdint.h>

typedef void (*VexDropFn)(void* data);

typedef struct VexArcHeader {
    volatile uint64_t strong_count;
    volatile uint64_t weak_count;
    uint32_t          data_offset;
    VexDropFn         drop_fn;
} VexArcHeader;

VexArcHeader* vex_arc_alloc(uint64_t data_size, uint64_t data_align, VexDropFn drop_fn);
void*         vex_arc_data(VexArcHeader* hdr);
VexArcHeader* vex_arc_clone(VexArcHeader* hdr);
void          vex_arc_drop(VexArcHeader* hdr);
VexArcHeader* vex_arc_weak_clone(VexArcHeader* hdr);
void          vex_arc_weak_drop(VexArcHeader* hdr);
VexArcHeader* vex_arc_weak_upgrade(VexArcHeader* hdr);

#ifdef __cplusplus
}
#endif
