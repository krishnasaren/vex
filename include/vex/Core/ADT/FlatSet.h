#pragma once
// include/vex/Core/ADT/FlatSet.h



// ============================================================================
// vex/Core/ADT/FlatSet.h  — Sorted flat vector-backed set (cache-friendly)
// ============================================================================
#include <algorithm>
#include <vector>

namespace vex {

template<typename T, typename Cmp = std::less<T>>
class FlatSet {
public:
    using iterator       = typename std::vector<T>::iterator;
    using const_iterator = typename std::vector<T>::const_iterator;

    bool   empty()  const { return data_.empty(); }
    size_t size()   const { return data_.size(); }

    bool insert(const T& v) {
        auto it = lower_bound(v);
        if (it != data_.end() && !Cmp{}(v, *it)) return false;
        data_.insert(it, v);
        return true;
    }

    bool contains(const T& v) const {
        auto it = lower_bound(v);
        return it != data_.end() && !Cmp{}(v, *it);
    }

    iterator       find(const T& v)       { auto it = lower_bound(v); return (it != data_.end() && !Cmp{}(v,*it)) ? it : data_.end(); }
    const_iterator find(const T& v) const { auto it = lower_bound(v); return (it != data_.end() && !Cmp{}(v,*it)) ? it : data_.end(); }

    void    erase(const T& v) { auto it = find(v); if (it != data_.end()) data_.erase(it); }
    void    clear() { data_.clear(); }

    iterator       begin()       { return data_.begin(); }
    iterator       end()         { return data_.end(); }
    const_iterator begin() const { return data_.begin(); }
    const_iterator end()   const { return data_.end(); }

private:
    std::vector<T> data_;
    Cmp            cmp_;

    const_iterator lower_bound(const T& v) const {
        return std::lower_bound(data_.begin(), data_.end(), v, cmp_);
    }
    iterator lower_bound(const T& v) {
        return std::lower_bound(data_.begin(), data_.end(), v, cmp_);
    }
};

} // namespace vex
