#pragma once
#ifdef __cplusplus
extern "C" {
#endif
#include <stdint.h>
uint64_t    vex_typeid_register(const char* name);
const char* vex_typeid_name(uint64_t id);
int         vex_typeid_eq(uint64_t a, uint64_t b);
#ifdef __cplusplus
}
#endif
