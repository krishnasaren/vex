#pragma once
#include <cstdint>
#include <string_view>

// Avoid clash with CMake -DVEX_VERSION_* defines by using an inline namespace trick.
// The CMake defines are integers; these are constexpr constants.
namespace vex {

#ifdef VEX_VERSION_MAJOR
static constexpr uint32_t VER_MAJOR = VEX_VERSION_MAJOR;
#undef VEX_VERSION_MAJOR
#else
static constexpr uint32_t VER_MAJOR = 1;
#endif

#ifdef VEX_VERSION_MINOR
static constexpr uint32_t VER_MINOR = VEX_VERSION_MINOR;
#undef VEX_VERSION_MINOR
#else
static constexpr uint32_t VER_MINOR = 0;
#endif

#ifdef VEX_VERSION_PATCH
static constexpr uint32_t VER_PATCH = VEX_VERSION_PATCH;
#undef VEX_VERSION_PATCH
#else
static constexpr uint32_t VER_PATCH = 0;
#endif

constexpr std::string_view VEX_VERSION_STRING      = "1.0.0";
constexpr std::string_view VEX_LANGUAGE_VERSION     = "4.0";
constexpr std::string_view VEX_TARGET_LLVM_VERSION  = "18";
constexpr uint32_t VXL_FORMAT_VERSION = 1;
constexpr uint32_t VXL_MAGIC = 0x5658'4C20u;

} // namespace vex
