// vex_rt/panic.c — VEX runtime panic handler

#include "vex_rt/panic.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <setjmp.h>

// Thread-local recovery stack for try/catch
// Each try block pushes a recovery point; pop on exit or panic
#define VEX_MAX_RECOVERY_DEPTH 64

typedef struct VexRecoveryPoint {
    jmp_buf         env;
    VexPanicInfo    info;
} VexRecoveryPoint;

// Thread-local recovery stack
#if defined(__GNUC__) || defined(__clang__)
static __thread VexRecoveryPoint vex_recovery_stack[VEX_MAX_RECOVERY_DEPTH];
static __thread int               vex_recovery_depth = 0;
#elif defined(_MSC_VER)
static __declspec(thread) VexRecoveryPoint vex_recovery_stack[VEX_MAX_RECOVERY_DEPTH];
static __declspec(thread) int               vex_recovery_depth = 0;
#else
// Single-threaded fallback
static VexRecoveryPoint vex_recovery_stack[VEX_MAX_RECOVERY_DEPTH];
static int               vex_recovery_depth = 0;
#endif

// ── vex_panic ─────────────────────────────────────────────────────────────────

void vex_panic(const char* message, const char* file, int line) {
    if (vex_recovery_depth > 0) {
        // Unwind to nearest try/catch
        VexRecoveryPoint* rp = &vex_recovery_stack[vex_recovery_depth - 1];
        rp->info.message    = message;
        rp->info.file       = file;
        rp->info.line       = line;
        rp->info.stackTrace = "(stack trace unavailable in this build)";
        longjmp(rp->env, 1);
        __builtin_unreachable();
    }

    // No recovery point — print and abort
    fprintf(stderr,
        "\n"
        "═══════════════════════════════════════════════════════\n"
        "  VEX RUNTIME PANIC\n"
        "  %s\n"
        "  At: %s:%d\n"
        "═══════════════════════════════════════════════════════\n"
        "\n",
        message ? message : "(no message)",
        file    ? file    : "(unknown file)",
        line
    );
    fflush(stderr);
    abort();
}

// ── vex_assert ────────────────────────────────────────────────────────────────

void vex_assert(int condition, const char* message, const char* file, int line) {
    if (!condition) {
        char buf[512];
        snprintf(buf, sizeof(buf), "Assertion failed: %s",
                 message ? message : "(no message)");
        vex_panic(buf, file, line);
    }
}

// ── Try/catch recovery point management ──────────────────────────────────────

VexPanicInfo* vex_try_begin(void) {
    if (vex_recovery_depth >= VEX_MAX_RECOVERY_DEPTH) {
        vex_panic("try/catch nesting too deep", __FILE__, __LINE__);
    }
    VexRecoveryPoint* rp = &vex_recovery_stack[vex_recovery_depth];
    memset(&rp->info, 0, sizeof(rp->info));
    vex_recovery_depth++;
    return &rp->info;
}

jmp_buf* vex_try_env(void) {
    if (vex_recovery_depth == 0) return NULL;
    return &vex_recovery_stack[vex_recovery_depth - 1].env;
}

void vex_try_end(void) {
    if (vex_recovery_depth > 0) vex_recovery_depth--;
}

VexPanicInfo* vex_catch_info(void) {
    if (vex_recovery_depth == 0) return NULL;
    return &vex_recovery_stack[vex_recovery_depth - 1].info;
}
