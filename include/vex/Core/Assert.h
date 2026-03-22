#pragma once
// ============================================================================
// vex/Core/Assert.h — Internal compiler assertion macros
// ============================================================================

#include <cstdlib>
#include <cstdio>
#include <string_view>

namespace vex {

// Called when an internal compiler assertion fails.
// Prints message and aborts. Never returns.
[[noreturn]] void assertionFailed(
    std::string_view msg,
    std::string_view file,
    int line,
    std::string_view func
);

} // namespace vex

// ── VEX_ASSERT — always-on internal compiler assertion ───────────────────────
// Use for invariants that MUST hold. These are compiler bugs, not user errors.
// They remain active in all build modes — compiler correctness is non-negotiable.

#define VEX_ASSERT(cond, msg)                                           \
    do {                                                                 \
        if (!(cond)) {                                                   \
            ::vex::assertionFailed(msg, __FILE__, __LINE__, __func__);  \
        }                                                                \
    } while (false)

// ── VEX_UNREACHABLE — marks code paths that must never execute ────────────────

#define VEX_UNREACHABLE(msg)                                            \
    ::vex::assertionFailed("Unreachable: " msg, __FILE__, __LINE__, __func__)

// ── VEX_UNIMPLEMENTED — stub for not-yet-written code ─────────────────────────

#define VEX_UNIMPLEMENTED(msg)                                          \
    ::vex::assertionFailed("Unimplemented: " msg, __FILE__, __LINE__, __func__)