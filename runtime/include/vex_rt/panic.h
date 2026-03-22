#pragma once
#ifdef __cplusplus
extern "C" {
#endif
#include <setjmp.h>

typedef struct {
    const char* message;
    const char* file;
    int         line;
    const char* stackTrace;
} VexPanicInfo;

void          vex_panic(const char* message, const char* file, int line);
void          vex_assert(int condition, const char* message, const char* file, int line);
VexPanicInfo* vex_try_begin(void);
jmp_buf*      vex_try_env(void);
void          vex_try_end(void);
VexPanicInfo* vex_catch_info(void);

#define VEX_PANIC(msg)          vex_panic((msg), __FILE__, __LINE__)
#define VEX_ASSERT(cond, msg)   vex_assert((int)(cond), (msg), __FILE__, __LINE__)

/* try/catch macros used by compiler-generated code */
#define VEX_TRY_BEGIN \
    do { \
        VexPanicInfo* _vex_pi = vex_try_begin(); \
        if (setjmp(*vex_try_env()) == 0) {

#define VEX_TRY_CATCH(var) \
        } else { \
            VexPanicInfo* var = vex_catch_info();

#define VEX_TRY_END \
        } \
        vex_try_end(); \
    } while(0)

#ifdef __cplusplus
}
#endif
