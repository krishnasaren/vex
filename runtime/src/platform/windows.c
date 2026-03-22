// vex_rt/platform/windows.c — Windows-specific runtime support
#include <windows.h>

int  vex_platform_getpid(void)  { return (int)GetCurrentProcessId(); }
int  vex_platform_getppid(void) { return 0; }
long vex_platform_pagesize(void){ SYSTEM_INFO si; GetSystemInfo(&si); return (long)si.dwPageSize; }
