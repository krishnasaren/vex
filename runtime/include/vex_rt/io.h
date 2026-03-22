#pragma once
#ifdef __cplusplus
extern "C" {
#endif
#include <stdint.h>

typedef struct { const char* ptr; uint64_t len; } VexStr;

void   vex_print(VexStr s);
void   vex_println(VexStr s);
void   vex_eprint(VexStr s);
void   vex_eprintln(VexStr s);
void   vex_flush_stdout(void);
void   vex_flush_stderr(void);
VexStr vex_readline(void);

#ifdef __cplusplus
}
#endif
