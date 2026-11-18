// src/Core/Assert.cpp
#include "vex/Core/Assert.h"
#include <cstdio>
#include <cstdlib>

namespace vex {

[[noreturn]] void assertionFailed(std::string_view msg,
                                  std::string_view file,
                                  int line,
                                  std::string_view func) {
    fprintf(stderr, "\n*** VEX compiler internal error ***\n");
    fprintf(stderr, "  Assertion: %.*s\n",  (int)msg.size(),  msg.data());
    fprintf(stderr, "  Function:  %.*s\n",  (int)func.size(), func.data());
    fprintf(stderr, "  Location:  %.*s:%d\n", (int)file.size(), file.data(), line);
    fprintf(stderr, "\nThis is a compiler bug. Please report it.\n\n");
    abort();
}

} // namespace vex
