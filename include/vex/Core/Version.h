#pragma once
// ============================================================================
// vex/Core/Version.h — VEX compiler version constants
// ============================================================================

#include <cstdint>
#include <string_view>

namespace vex {

// Compiler version
constexpr uint32_t VEX_VERSION_MAJOR = 1;
constexpr uint32_t VEX_VERSION_MINOR = 0;
constexpr uint32_t VEX_VERSION_PATCH = 0;

constexpr std::string_view VEX_VERSION_STRING = "1.0.0";
constexpr std::string_view VEX_LANGUAGE_VERSION = "4.0";

// Language feature flags
constexpr std::string_view VEX_TARGET_LLVM_VERSION = "17.0.6";

// VXL binary format version
constexpr uint32_t VXL_FORMAT_VERSION = 1;
constexpr uint32_t VXL_MAGIC = 0x5658_4C20; // "VXL "

} // namespace vex