// vex_rt/platform/wasm.c — WebAssembly runtime support
int  vex_platform_getpid(void)  { return 1; }
int  vex_platform_getppid(void) { return 0; }
long vex_platform_pagesize(void){ return 65536; } /* WASM page = 64 KB */
