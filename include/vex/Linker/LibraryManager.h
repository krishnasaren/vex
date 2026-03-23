#pragma once
// include/vex/Linker/LibraryManager.h


// vex/Linker/LibraryManager.h  — Manages library search paths
#include <string>
#include <vector>
#include <string_view>
namespace vex {
class LibraryManager {
public:
    void addSearchPath(std::string_view path) { paths_.emplace_back(path); }

    // Resolve -lname to a full path (searches static then dynamic)
    std::string resolve(std::string_view libName) const;

    // Resolve .vxl precompiled module
    std::string resolveModule(std::string_view modName) const;

    const std::vector<std::string>& paths() const { return paths_; }

private:
    std::vector<std::string> paths_;
};
} // namespace vex
