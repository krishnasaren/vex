// ============================================================================
// vex/Core/Platform.cpp
// ============================================================================

#include "vex/Core/Platform.h"
#include <cassert>

namespace vex {

std::string_view targetOSName(TargetOS os) {
    switch (os) {
        case TargetOS::Linux:   return "linux";
        case TargetOS::Windows: return "windows";
        case TargetOS::MacOS:   return "macos";
        case TargetOS::Android: return "android";
        case TargetOS::Wasm:    return "wasm";
        case TargetOS::Unknown: return "unknown";
    }
    return "unknown";
}

std::string_view targetArchName(TargetArch arch) {
    switch (arch) {
        case TargetArch::X86_64:  return "x86_64";
        case TargetArch::AArch64: return "arm64";
        case TargetArch::RISCV64: return "riscv64";
        case TargetArch::Wasm32:  return "wasm32";
        case TargetArch::Unknown: return "unknown";
    }
    return "unknown";
}

TargetOS parseTargetOS(std::string_view name) {
    if (name == "linux")   return TargetOS::Linux;
    if (name == "windows") return TargetOS::Windows;
    if (name == "macos")   return TargetOS::MacOS;
    if (name == "android") return TargetOS::Android;
    if (name == "wasm")    return TargetOS::Wasm;
    return TargetOS::Unknown;
}

TargetArch parseTargetArch(std::string_view name) {
    if (name == "x86_64")  return TargetArch::X86_64;
    if (name == "arm64")   return TargetArch::AArch64;
    if (name == "aarch64") return TargetArch::AArch64;
    if (name == "riscv64") return TargetArch::RISCV64;
    if (name == "wasm32")  return TargetArch::Wasm32;
    return TargetArch::Unknown;
}

bool is64Bit(TargetArch arch) {
    switch (arch) {
        case TargetArch::X86_64:  return true;
        case TargetArch::AArch64: return true;
        case TargetArch::RISCV64: return true;
        case TargetArch::Wasm32:  return false;
        case TargetArch::Unknown: return true; // assume 64-bit
    }
    return true;
}

uint32_t nativeIntSize(TargetArch arch) {
    return is64Bit(arch) ? 8 : 4;
}

} // namespace vex