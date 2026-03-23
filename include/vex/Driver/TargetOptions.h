#pragma once
// include/vex/Driver/TargetOptions.h


// vex/Driver/TargetOptions.h  — Fine-grained code-generation options
#include <string>
#include <vector>
namespace vex {
struct TargetOptions {
    std::string cpu;              // e.g. "znver3"
    std::vector<std::string> features; // e.g. "+avx2", "-sse4.1"
    bool relocPIC  = false;       // Position-independent code
    bool softFloat = false;
    bool noRedZone = false;
    bool stackRealign = false;
    std::string abi;              // e.g. "sysv", "win64", "aapcs64"
};
} // namespace vex
