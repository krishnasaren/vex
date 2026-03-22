#pragma once
#ifdef __cplusplus
extern "C" {
#endif
#include <stdint.h>
#include <pthread.h>
#if defined(__linux__)
#  include <sched.h>
#  include <unistd.h>
#endif

typedef void* (*VexThreadFn)(void* arg);
typedef struct { pthread_t inner; } VexThreadHandle;

VexThreadHandle vex_thread_spawn(VexThreadFn fn, void* arg, uint64_t stack_size);
void*           vex_thread_join(VexThreadHandle h);
void            vex_thread_detach(VexThreadHandle h);
uint64_t        vex_thread_id(void);
void            vex_thread_yield(void);
void            vex_thread_sleep_ns(uint64_t ns);
uint32_t        vex_hardware_concurrency(void);

#ifdef __cplusplus
}
#endif
