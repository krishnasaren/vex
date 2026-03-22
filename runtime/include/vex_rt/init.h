#pragma once
#ifdef __cplusplus
extern "C" {
#endif
#include <stdint.h>
typedef void (*VexModuleInitFn)(void);
typedef void (*VexModuleDeinitFn)(void);
void vex_register_init(VexModuleInitFn fn);
void vex_register_deinit(VexModuleDeinitFn fn);
void vex_rt_init(uint32_t async_workers);
void vex_rt_deinit(void);
#ifdef __cplusplus
}
#endif
