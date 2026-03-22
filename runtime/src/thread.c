// vex_rt/thread.c — OS thread management

#include "vex_rt/thread.h"
#include "vex_rt/alloc.h"
#include "vex_rt/panic.h"
#include <string.h>
#include <time.h>

typedef struct { VexThreadFn fn; void* arg; } VexThreadCtx;

static void* vex_thread_trampoline(void* raw) {
    VexThreadCtx* ctx = (VexThreadCtx*)raw;
    void* result = ctx->fn(ctx->arg);
    vex_free(ctx);
    return result;
}

VexThreadHandle vex_thread_spawn(VexThreadFn fn, void* arg, uint64_t stack_size) {
    VexThreadHandle h;
    pthread_attr_t attr;
    pthread_attr_init(&attr);
    if (stack_size > 0) pthread_attr_setstacksize(&attr, (size_t)stack_size);

    VexThreadCtx* ctx = (VexThreadCtx*)vex_alloc(sizeof(VexThreadCtx), _Alignof(VexThreadCtx));
    ctx->fn  = fn;
    ctx->arg = arg;

    if (pthread_create(&h.inner, &attr, vex_thread_trampoline, ctx) != 0)
        vex_panic("vex_thread_spawn: pthread_create failed", __FILE__, __LINE__);

    pthread_attr_destroy(&attr);
    return h;
}

void* vex_thread_join(VexThreadHandle h) {
    void* retval = NULL;
    pthread_join(h.inner, &retval);
    return retval;
}

void vex_thread_detach(VexThreadHandle h) { pthread_detach(h.inner); }

uint64_t vex_thread_id(void) {
#if defined(__linux__)
    return (uint64_t)pthread_self();
#elif defined(__APPLE__)
    uint64_t tid = 0;
    pthread_threadid_np(NULL, &tid);
    return tid;
#else
    return (uint64_t)(uintptr_t)pthread_self();
#endif
}

void vex_thread_yield(void) { sched_yield(); }

void vex_thread_sleep_ns(uint64_t ns) {
    struct timespec ts;
    ts.tv_sec  = (time_t)(ns / 1000000000ULL);
    ts.tv_nsec = (long)(ns % 1000000000ULL);
    nanosleep(&ts, NULL);
}

uint32_t vex_hardware_concurrency(void) {
#if defined(_SC_NPROCESSORS_ONLN)
    long n = sysconf(_SC_NPROCESSORS_ONLN);
    return (n > 0) ? (uint32_t)n : 1u;
#elif defined(_WIN32)
    SYSTEM_INFO si; GetSystemInfo(&si); return (uint32_t)si.dwNumberOfProcessors;
#else
    return 1u;
#endif
}
