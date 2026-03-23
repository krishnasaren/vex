#pragma once
// include/vex/Driver/BuildMode.h


#pragma once
// vex/Driver/BuildMode.h
#include <string_view>
namespace vex {
enum class BuildMode : uint8_t { Debug, Release, RelWithDebInfo, MinSizeRel };
inline std::string_view buildModeName(BuildMode m) {
    switch (m) {
        case BuildMode::Debug:          return "debug";
        case BuildMode::Release:        return "release";
        case BuildMode::RelWithDebInfo: return "relwithdebinfo";
        case BuildMode::MinSizeRel:     return "minsizerel";
    }
    return "unknown";
}
} // namespace vex





