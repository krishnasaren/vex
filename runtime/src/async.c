// vex_rt/async.c — Minimal cooperative async task executor (M:N scheduler stub)
// Full implementation requires platform-specific event loops (io_uring/kqueue/IOCP).
// This stub provides the API surface so the compiler can link against it.

#include "vex_rt/async.h"
#include "vex_rt/alloc.h"
#include "vex_rt/panic.h"
#include "vex_rt/thread.h"
#include "vex_rt/sync.h"
#include <string.h>
#include <stdlib.h>

/* ── Task queue ──────────────────────────────────────────────────────────────*/
#define VEX_MAX_TASKS 65536

typedef struct {
    VexTaskFn   fn;
    void*       arg;
    void*       result;
    int         done;
    VexMutex    mutex;
} VexTask;

typedef struct {
    VexTask*    tasks[VEX_MAX_TASKS];
    uint64_t    head, tail, count;
    VexMutex    mutex;
    VexSemaphore sem;
    int         shutdown;
    uint32_t    num_workers;
    VexThreadHandle workers[64];
} VexExecutor;

static VexExecutor* g_executor = NULL;

static void* executor_worker(void* arg) {
    VexExecutor* ex = (VexExecutor*)arg;
    while (1) {
        vex_semaphore_acquire(&ex->sem);
        vex_mutex_lock(&ex->mutex);
        if (ex->shutdown && ex->count == 0) {
            vex_mutex_unlock(&ex->mutex);
            break;
        }
        VexTask* task = NULL;
        if (ex->count > 0) {
            task = ex->tasks[ex->head % VEX_MAX_TASKS];
            ex->head++; ex->count--;
        }
        vex_mutex_unlock(&ex->mutex);
        if (task) {
            task->result = task->fn(task->arg);
            vex_mutex_lock(&task->mutex);
            task->done = 1;
            vex_mutex_unlock(&task->mutex);
        }
    }
    return NULL;
}

void vex_async_init(uint32_t num_workers) {
    if (g_executor) return;
    if (num_workers == 0) num_workers = vex_hardware_concurrency();
    if (num_workers > 64) num_workers = 64;

    g_executor = (VexExecutor*)vex_alloc(sizeof(VexExecutor), _Alignof(VexExecutor));
    memset(g_executor, 0, sizeof(VexExecutor));
    vex_mutex_init(&g_executor->mutex);
    vex_semaphore_init(&g_executor->sem, 0);
    g_executor->num_workers = num_workers;

    for (uint32_t i = 0; i < num_workers; i++)
        g_executor->workers[i] = vex_thread_spawn(executor_worker, g_executor, 0);
}

void vex_async_shutdown(void) {
    if (!g_executor) return;
    vex_mutex_lock(&g_executor->mutex);
    g_executor->shutdown = 1;
    vex_mutex_unlock(&g_executor->mutex);
    for (uint32_t i = 0; i < g_executor->num_workers; i++)
        vex_semaphore_release(&g_executor->sem);
    for (uint32_t i = 0; i < g_executor->num_workers; i++)
        vex_thread_join(g_executor->workers[i]);
    vex_semaphore_destroy(&g_executor->sem);
    vex_mutex_destroy(&g_executor->mutex);
    vex_free(g_executor);
    g_executor = NULL;
}

VexTask* vex_async_spawn(VexTaskFn fn, void* arg) {
    if (!g_executor) vex_async_init(0);
    VexTask* t = (VexTask*)vex_alloc(sizeof(VexTask), _Alignof(VexTask));
    t->fn = fn; t->arg = arg; t->result = NULL; t->done = 0;
    vex_mutex_init(&t->mutex);
    vex_mutex_lock(&g_executor->mutex);
    if (g_executor->count >= VEX_MAX_TASKS)
        vex_panic("vex_async_spawn: task queue full", __FILE__, __LINE__);
    g_executor->tasks[g_executor->tail % VEX_MAX_TASKS] = t;
    g_executor->tail++; g_executor->count++;
    vex_mutex_unlock(&g_executor->mutex);
    vex_semaphore_release(&g_executor->sem);
    return t;
}

void* vex_async_join(VexTask* t) {
    if (!t) return NULL;
    /* Spin-wait (production impl uses condition variables) */
    while (1) {
        vex_mutex_lock(&t->mutex);
        if (t->done) { void* r = t->result; vex_mutex_unlock(&t->mutex); vex_free(t); return r; }
        vex_mutex_unlock(&t->mutex);
        vex_thread_sleep_ns(1000000); /* 1ms */
    }
}

void* vex_async_run(VexTaskFn fn, void* arg) {
    VexTask* t = vex_async_spawn(fn, arg);
    return vex_async_join(t);
}

void vex_async_sleep_ns(uint64_t ns) { vex_thread_sleep_ns(ns); }
