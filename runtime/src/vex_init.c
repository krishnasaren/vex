/* vex_init.c — Runtime initialization and global state */
#include "../include/vex_rt.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

static int    _vex_argc = 0;
static char** _vex_argv = NULL;

void vex_rt_init(int argc, char** argv) {
    _vex_argc = argc;
    _vex_argv = argv;
}

void vex_rt_fini(void) {
    fflush(stdout);
    fflush(stderr);
}

VexVec vex_env_args(void) {
    VexVec v = vex_vec_new(sizeof(VexStr));
    for (int i = 0; i < _vex_argc; i++) {
        VexStr s = vex_str_from_cstr(_vex_argv[i]);
        vex_vec_push(&v, &s);
    }
    return v;
}

VexStr vex_env_get(VexStr key) {
    /* Null-terminate the key */
    char buf[512];
    size_t n = key.len < 511 ? key.len : 511;
    memcpy(buf, key.ptr, n);
    buf[n] = '\0';
    const char* val = getenv(buf);
    if (!val) return (VexStr){.ptr = "", .len = 0};
    return vex_str_from_cstr(val);
}

_Noreturn void vex_process_exit(int32_t code) {
    vex_rt_fini();
    exit((int)code);
}
