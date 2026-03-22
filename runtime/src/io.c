// vex_rt/io.c — I/O primitives

#include "vex_rt/io.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

/* VexStr is a fat pointer { ptr: *uint8, len: uint64 } */
typedef struct { const char* ptr; uint64_t len; } VexStr;

void vex_print(VexStr s)   { fwrite(s.ptr, 1, (size_t)s.len, stdout); }
void vex_println(VexStr s) { fwrite(s.ptr, 1, (size_t)s.len, stdout); fputc('\n', stdout); }
void vex_eprint(VexStr s)  { fwrite(s.ptr, 1, (size_t)s.len, stderr); }
void vex_eprintln(VexStr s){ fwrite(s.ptr, 1, (size_t)s.len, stderr); fputc('\n', stderr); }
void vex_flush_stdout(void){ fflush(stdout); }
void vex_flush_stderr(void){ fflush(stderr); }

/* Read one line from stdin; returns heap-allocated VexStr. Caller frees ptr. */
VexStr vex_readline(void) {
    char*  buf  = NULL;
    size_t cap  = 0;
    ssize_t len = -1;
#if defined(_WIN32)
    /* Windows fallback */
    char tmp[4096];
    if (fgets(tmp, sizeof(tmp), stdin)) {
        len = (ssize_t)strlen(tmp);
        if (len > 0 && tmp[len-1] == '\n') { tmp[len-1] = '\0'; len--; }
        buf = (char*)malloc((size_t)(len + 1));
        memcpy(buf, tmp, (size_t)(len + 1));
    }
#else
    len = getline(&buf, &cap, stdin);
    if (len > 0 && buf[len-1] == '\n') { buf[len-1] = '\0'; len--; }
    if (len < 0) { free(buf); buf = NULL; len = 0; }
#endif
    VexStr s; s.ptr = buf ? buf : ""; s.len = (uint64_t)(len < 0 ? 0 : len);
    return s;
}
