// vex_rt/init.c — Module initialisation/deinitialisation

#include "vex_rt/init.h"
#include "vex_rt/async.h"
#include "vex_rt/alloc.h"
#include <stdlib.h>

static VexModuleInitFn  g_inits[1024];
static VexModuleDeinitFn g_deinits[1024];
static int g_init_count   = 0;
static int g_deinit_count = 0;

void vex_register_init(VexModuleInitFn fn)     { if (g_init_count   < 1024) g_inits[g_init_count++]     = fn; }
void vex_register_deinit(VexModuleDeinitFn fn) { if (g_deinit_count < 1024) g_deinits[g_deinit_count++] = fn; }

void vex_rt_init(uint32_t async_workers) {
    vex_async_init(async_workers);
    for (int i = 0; i < g_init_count; i++)
        if (g_inits[i]) g_inits[i]();
}

void vex_rt_deinit(void) {
    for (int i = g_deinit_count - 1; i >= 0; i--)
        if (g_deinits[i]) g_deinits[i]();
    vex_async_shutdown();
}
