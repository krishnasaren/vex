#include "vex_rt/stackguard.h"
#include "vex_rt/panic.h"
#include <signal.h>
#include <string.h>
static void stack_overflow_handler(int sig) {
    (void)sig;
    vex_panic("stack overflow", __FILE__, __LINE__);
}
void vex_stackguard_init(void) {
#if defined(__linux__) || defined(__APPLE__)
    struct sigaction sa;
    memset(&sa, 0, sizeof(sa));
    sa.sa_handler = stack_overflow_handler;
    sa.sa_flags   = SA_ONSTACK;
    sigaction(SIGSEGV, &sa, NULL);
#endif
}
