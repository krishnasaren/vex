/* vex_panic.c — Panic and runtime error handlers */
#include "../include/vex_rt.h"
#include <stdio.h>
#include <stdlib.h>

_Noreturn void vex_panic(const char* msg, size_t msg_len,
                          const char* file, uint32_t line) {
    fprintf(stderr, "\n\033[1;31mpanic\033[0m: %.*s\n",
            (int)msg_len, msg);
    if (file)
        fprintf(stderr, "  at %s:%u\n", file, line);
    fflush(stderr);
    abort();
}

_Noreturn void vex_panic_oob(size_t idx, size_t len,
                              const char* file, uint32_t line) {
    fprintf(stderr,
            "\n\033[1;31mpanic\033[0m: index out of bounds: "
            "index %zu, length %zu\n  at %s:%u\n",
            idx, len, file ? file : "?", line);
    fflush(stderr);
    abort();
}

_Noreturn void vex_panic_null(const char* file, uint32_t line) {
    fprintf(stderr,
            "\n\033[1;31mpanic\033[0m: null pointer dereference\n"
            "  at %s:%u\n",
            file ? file : "?", line);
    fflush(stderr);
    abort();
}

_Noreturn void vex_panic_overflow(const char* file, uint32_t line) {
    fprintf(stderr,
            "\n\033[1;31mpanic\033[0m: integer overflow\n"
            "  at %s:%u\n",
            file ? file : "?", line);
    fflush(stderr);
    abort();
}

_Noreturn void vex_unreachable(const char* file, uint32_t line) {
    fprintf(stderr,
            "\n\033[1;31minternal error\033[0m: entered unreachable code\n"
            "  at %s:%u\n",
            file ? file : "?", line);
    fflush(stderr);
    abort();
}
