#pragma once
// ============================================================================
// vex/Core/Platform.h — Platform detection and OS/arch constants
// ============================================================================

#include <cstdint>
#include <string_view>

namespace vex {

// ── OS Detection ─────────────────────────────────────────────────────────────

#if defined(_WIN32) || defined(_WIN64)
    #define VEX_OS_WINDOWS 1
    constexpr std::string_view VEX_HOST_OS = "windows";
#elif defined(__APPLE__)
    #define VEX_OS_MACOS 1
    constexpr std::string_view VEX_HOST_OS = "macos";
#elif defined(__linux__)
    #define VEX_OS_LINUX 1
    constexpr std::string_view VEX_HOST_OS = "linux";
#elif defined(__wasm__)
    #define VEX_OS_WASM 1
    constexpr std::string_view VEX_HOST_OS = "wasm";
#else
    #define VEX_OS_UNKNOWN 1
    constexpr std::string_view VEX_HOST_OS = "unknown";
#endif

// ── Architecture Detection ───────────────────────────────────────────────────

#if defined(__x86_64__) || defined(_M_X64)
    #define VEX_ARCH_X86_64 1
    constexpr std::string_view VEX_HOST_ARCH = "x86_64";
    constexpr uint32_t VEX_POINTER_SIZE = 8;
#elif defined(__aarch64__) || defined(_M_ARM64)
    #define VEX_ARCH_AARCH64 1
    constexpr std::string_view VEX_HOST_ARCH = "arm64";
    constexpr uint32_t VEX_POINTER_SIZE = 8;
#elif defined(__wasm32__)
    #define VEX_ARCH_WASM32 1
    constexpr std::string_view VEX_HOST_ARCH = "wasm32";
    constexpr uint32_t VEX_POINTER_SIZE = 4;
#else
    constexpr std::string_view VEX_HOST_ARCH = "unknown";
    constexpr uint32_t VEX_POINTER_SIZE = sizeof(void*);
#endif

// ── Target OS enum ───────────────────────────────────────────────────────────
// These match the VEX language #OS comptime variable values

enum class TargetOS : uint8_t {
    Linux,
    Windows,
    MacOS,
    Android,
    Wasm,
    Unknown
};

// ── Target Architecture enum ─────────────────────────────────────────────────
// These match the VEX language #ARCH comptime variable values

enum class TargetArch : uint8_t {
    X86_64,
    AArch64,
    RISCV64,
    Wasm32,
    Unknown
};

// ── Utility functions ────────────────────────────────────────────────────────

std::string_view targetOSName(TargetOS os);
std::string_view targetArchName(TargetArch arch);
TargetOS parseTargetOS(std::string_view name);
TargetArch parseTargetArch(std::string_view name);

// Returns true if the target requires 64-bit pointer size
bool is64Bit(TargetArch arch);

// Returns the natural int/uint size for the target (8 bytes on 64-bit)
uint32_t nativeIntSize(TargetArch arch);

} // namespace vex