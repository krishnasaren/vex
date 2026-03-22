#include "vex_rt/typeid.h"
#include <string.h>

static const char* vex_typeid_name_table[65536];
static uint64_t    vex_typeid_next = 1;

uint64_t vex_typeid_register(const char* name) {
    uint64_t id = vex_typeid_next++;
    if (id < 65536) vex_typeid_name_table[id] = name;
    return id;
}

const char* vex_typeid_name(uint64_t id) {
    if (id < 65536 && vex_typeid_name_table[id]) return vex_typeid_name_table[id];
    return "<unknown>";
}

int vex_typeid_eq(uint64_t a, uint64_t b) { return a == b; }
