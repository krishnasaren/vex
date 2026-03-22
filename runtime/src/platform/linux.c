// vex_rt/platform/linux.c — Linux-specific runtime support
#include <unistd.h>
#include <sys/types.h>

int   vex_platform_getpid(void)  { return (int)getpid(); }
int   vex_platform_getppid(void) { return (int)getppid(); }
long  vex_platform_pagesize(void){ return sysconf(_SC_PAGESIZE); }
