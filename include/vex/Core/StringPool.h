#pragma once
// include/vex/Core/StringPool.h



// ============================================================================
// vex/Core/StringPool.h  — Global string interning pool
// ============================================================================
#include "vex/Core/InternedString.h"
#include "vex/Core/BumpAllocator.h"
#include <string_view>
#include <unordered_map>

namespace vex {

class StringPool {
public:
    explicit StringPool(BumpAllocator& alloc) : alloc_(alloc) {}

    // Intern a string; returns a stable pointer into the arena
    InternedString intern(std::string_view s);

    size_t size() const { return table_.size(); }

private:
    BumpAllocator& alloc_;
    std::unordered_map<std::string_view, InternedString> table_;
};

} // namespace vex
