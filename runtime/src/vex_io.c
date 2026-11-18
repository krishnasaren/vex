/* vex_io.c — I/O runtime functions */
#include "../include/vex_rt.h"
#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>
#include <string.h>

void vex_print(VexStr s) {
    fwrite(s.ptr, 1, s.len, stdout);
}

void vex_println(VexStr s) {
    fwrite(s.ptr, 1, s.len, stdout);
    fputc('\n', stdout);
}

void vex_eprint(VexStr s) {
    fwrite(s.ptr, 1, s.len, stderr);
}

void vex_eprintln(VexStr s) {
    fwrite(s.ptr, 1, s.len, stderr);
    fputc('\n', stderr);
}

void vex_print_i64(int64_t v)  { printf("%lld",  (long long)v); }
void vex_print_u64(uint64_t v) { printf("%llu", (unsigned long long)v); }
void vex_print_f64(double v)   { printf("%.17g", v); }
void vex_print_bool(bool v)    { fputs(v ? "true" : "false", stdout); }
void vex_print_char(uint32_t cp) {
    /* Encode as UTF-8 and print */
    if (cp < 0x80) {
        putchar((int)cp);
    } else if (cp < 0x800) {
        putchar((int)(0xC0 | (cp >> 6)));
        putchar((int)(0x80 | (cp & 0x3F)));
    } else if (cp < 0x10000) {
        putchar((int)(0xE0 | (cp >> 12)));
        putchar((int)(0x80 | ((cp >> 6) & 0x3F)));
        putchar((int)(0x80 | (cp & 0x3F)));
    } else {
        putchar((int)(0xF0 | (cp >> 18)));
        putchar((int)(0x80 | ((cp >> 12) & 0x3F)));
        putchar((int)(0x80 | ((cp >> 6)  & 0x3F)));
        putchar((int)(0x80 | (cp & 0x3F)));
    }
}

void vex_flush_stdout(void) { fflush(stdout); }
void vex_flush_stderr(void) { fflush(stderr); }

VexString vex_format(const char* fmt, ...) {
    va_list ap;
    va_start(ap, fmt);
    int n = vsnprintf(NULL, 0, fmt, ap);
    va_end(ap);
    if (n < 0) return vex_string_new();

    VexString s = vex_string_with_capacity((size_t)(n + 1));
    va_start(ap, fmt);
    vsnprintf(s.ptr, (size_t)(n + 1), fmt, ap);
    va_end(ap);
    s.len = (size_t)n;
    return s;
}
