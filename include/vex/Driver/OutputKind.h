#pragma once
// include/vex/Driver/OutputKind.h


// vex/Driver/OutputKind.h
#include <string_view>
namespace vex {
enum class OutputKind : uint8_t {
    Executable,      // default
    SharedLibrary,   // -shared
    StaticLibrary,   // -static
    ObjectFile,      // -c
    LLVMIR,          // -emit-llvm
    LLVMBC,          // -emit-bc
    Assembly,        // -S
    VexIR,           // -emit-vexir
};
std::string_view outputKindName(OutputKind k);
std::string_view outputKindExt(OutputKind k);
} // namespace vex
