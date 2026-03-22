// ============================================================================
// vex/Core/Assert.cpp
// ============================================================================

#include "vex/Core/Assert.h"
#include <cstdio>
#include <cstdlib>

namespace vex {

[[noreturn]] void assertionFailed(
    std::string_view msg,
    std::string_view file,
    int line,
    std::string_view func)
{
    fprintf(stderr,
        "\n"
        "=== VEX INTERNAL COMPILER ERROR ===\n"
        "  Assertion failed: %.*s\n"
        "  Location: %.*s:%d\n"
        "  Function: %.*s\n"
        "\n"
        "This is a compiler bug. Please report it.\n"
        "\n",
        (int)msg.size(),  msg.data(),
        (int)file.size(), file.data(),
        line,
        (int)func.size(), func.data()
    );
    fflush(stderr);
    abort();
}

} // namespace vex