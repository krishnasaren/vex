// src/Core/StringPool.cpp


#include "vex/Core/StringPool.h"
#include <cstring>

namespace vex {

InternedString StringPool::intern(std::string_view s) {
    auto it = table_.find(s);
    if (it != table_.end())
        return it->second;

    // Copy string into the arena
    char* p = static_cast<char*>(alloc_.allocate(s.size() + 1, 1));
    memcpy(p, s.data(), s.size());
    p[s.size()] = '\0';

    InternedString is(p, s.size());
    // Key into map using the stable pointer
    table_.emplace(std::string_view(p, s.size()), is);
    return is;
}

} // namespace vex
