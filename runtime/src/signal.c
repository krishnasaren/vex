#include "vex_rt/signal.h"
#include <signal.h>

void vex_signal(int signum, VexSignalHandler handler) { signal(signum, handler); }
void vex_ignore_signal(int signum)  { signal(signum, SIG_IGN); }
void vex_default_signal(int signum) { signal(signum, SIG_DFL); }
void vex_raise_signal(int signum)   { raise(signum); }
