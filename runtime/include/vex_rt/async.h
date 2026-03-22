#pragma once
#ifdef __cplusplus
extern "C" {
#endif
#include <stdint.h>

typedef void* (*VexTaskFn)(void* arg);
typedef struct VexTask VexTask;

void      vex_async_init(uint32_t num_workers);
void      vex_async_shutdown(void);
VexTask*  vex_async_spawn(VexTaskFn fn, void* arg);
void*     vex_async_join(VexTask* task);
void*     vex_async_run(VexTaskFn fn, void* arg);
void      vex_async_sleep_ns(uint64_t ns);

#ifdef __cplusplus
}
#endif
