#pragma once
// include/vex/Core/ADT/ScopedHashTable.h


// ============================================================================
// vex/Core/ADT/ScopedHashTable.h  — Hash table with scope-based rollback
// ============================================================================
#include <functional>
#include <string>
#include <unordered_map>
#include <vector>

namespace vex {

template<typename K, typename V, typename Hash = std::hash<K>>
class ScopedHashTable {
public:
    struct Entry { K key; V value; bool hadPrev; V prev; };

    void pushScope() { trail_.push_back(trailMark_{}); }

    void popScope() {
        while (!trail_.empty()) {
            auto& e = trail_.back();
            if (std::holds_alternative<trailMark_>(e)) { trail_.pop_back(); break; }
            auto& ent = std::get<Entry>(e);
            if (ent.hadPrev) table_[ent.key] = ent.prev;
            else             table_.erase(ent.key);
            trail_.pop_back();
        }
    }

    void insert(const K& k, V v) {
        Entry e;
        e.key = k; e.value = v;
        auto it = table_.find(k);
        e.hadPrev = (it != table_.end());
        if (e.hadPrev) e.prev = it->second;
        trail_.push_back(e);
        table_[k] = std::move(v);
    }

    V*       lookup(const K& k)       { auto it = table_.find(k); return it != table_.end() ? &it->second : nullptr; }
    const V* lookup(const K& k) const { auto it = table_.find(k); return it != table_.end() ? &it->second : nullptr; }

    bool contains(const K& k) const { return table_.count(k) > 0; }

private:
    struct trailMark_ {};
    std::unordered_map<K,V,Hash>  table_;
    std::vector<std::variant<Entry, trailMark_>> trail_;
};

} // namespace vex
