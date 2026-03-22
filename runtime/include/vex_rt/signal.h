#pragma once
#ifdef __cplusplus
extern "C" {
#endif
typedef void (*VexSignalHandler)(int);
void vex_signal(int signum, VexSignalHandler handler);
void vex_ignore_signal(int signum);
void vex_default_signal(int signum);
void vex_raise_signal(int signum);
#ifdef __cplusplus
}
#endif
