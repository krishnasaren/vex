#pragma once
// include/vex/Core/InternedString.h

// ============================================================================
// vex/Core/InternedString.h  — Interned (deduplicated) string type
// ============================================================================
#include <string_view>
#include <cstdint>

namespace vex {

// An InternedString is an index into the global string pool.
// Equality is O(1) pointer comparison.
class InternedString {
public:
    InternedString() = default;
    explicit InternedString(const char* data, size_t len)
        : data_(data), len_(len) {}

    std::string_view view() const { return {data_, len_}; }
    const char*      data() const { return data_; }
    size_t           size() const { return len_; }
    bool             empty() const { return len_ == 0; }

    bool operator==(InternedString o) const { return data_ == o.data_; }
    bool operator!=(InternedString o) const { return data_ != o.data_; }
    bool operator< (InternedString o) const { return data_ <  o.data_; }

    static InternedString empty_str();

private:
    const char* data_ = nullptr;
    size_t      len_  = 0;
};

} // namespace vex

// Hash support
namespace std {
template<> struct hash<vex::InternedString> {
    size_t operator()(vex::InternedString s) const noexcept {
        return hash<const char*>{}(s.data());
    }
};
} // namespace std
