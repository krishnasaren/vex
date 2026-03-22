#pragma once
// ============================================================================
// vex/AST/ASTContext.h
// Owns all AST node memory via a bump allocator.
// All nodes are allocated here and live until the ASTContext is destroyed.
// No individual node deletes — entire context freed at once.
// ============================================================================

#include "vex/AST/ASTNode.h"
#include <cstddef>
#include <cstdlib>
#include <memory>
#include <vector>
#include <string>
#include <string_view>
#include <cstring>

namespace vex {

// ── ArenaAllocator ────────────────────────────────────────────────────────────
// Simple bump pointer allocator. Very fast allocation, bulk free.

class ArenaAllocator {
public:
    explicit ArenaAllocator(size_t blockSize = 1024 * 1024) // 1MB blocks
        : blockSize_(blockSize) {
        allocNewBlock();
    }

    ~ArenaAllocator() {
        for (auto* block : blocks_)
            ::free(block);
    }

    // Allocate n bytes aligned to `align` bytes.
    void* allocate(size_t n, size_t align = alignof(std::max_align_t)) {
        // Align current pointer
        size_t rem = reinterpret_cast<uintptr_t>(cur_) % align;
        if (rem != 0) cur_ += align - rem;

        if (cur_ + n > end_)
            allocNewBlock(n > blockSize_ ? n : blockSize_);

        void* ptr = cur_;
        cur_ += n;
        return ptr;
    }

    // Allocate and zero
    void* allocateZero(size_t n, size_t align = alignof(std::max_align_t)) {
        void* ptr = allocate(n, align);
        std::memset(ptr, 0, n);
        return ptr;
    }

    // Total bytes allocated (approx)
    size_t bytesAllocated() const {
        size_t total = 0;
        for (auto* b : blocks_)
            total += blockSize_;
        return total;
    }

private:
    std::vector<char*> blocks_;
    char*              cur_  = nullptr;
    char*              end_  = nullptr;
    size_t             blockSize_;

    void allocNewBlock(size_t size = 0) {
        size_t bs = size > 0 ? size : blockSize_;
        char* block = static_cast<char*>(::malloc(bs));
        blocks_.push_back(block);
        cur_ = block;
        end_ = block + bs;
    }
};

// ── ASTContext ────────────────────────────────────────────────────────────────

class ASTContext {
public:
    ASTContext() = default;
    ~ASTContext() = default;

    // Non-copyable
    ASTContext(const ASTContext&) = delete;
    ASTContext& operator=(const ASTContext&) = delete;

    // ── Node allocation ───────────────────────────────────────────────────────

    // Allocate and construct any AST node type.
    // The node is owned by this ASTContext and freed when it is destroyed.
    template<typename T, typename... Args>
    T* make(Args&&... args) {
        static_assert(std::is_base_of_v<ASTNode, T>,
                      "ASTContext::make() only creates ASTNode subclasses");
        void* mem = arena_.allocate(sizeof(T), alignof(T));
        return new (mem) T(std::forward<Args>(args)...);
    }

    // ── String interning ──────────────────────────────────────────────────────
    // Store strings in the arena for zero-copy string_view usage.

    std::string_view intern(std::string_view s) {
        char* buf = static_cast<char*>(arena_.allocate(s.size() + 1, 1));
        std::memcpy(buf, s.data(), s.size());
        buf[s.size()] = '\0';
        return {buf, s.size()};
    }

    // ── Vector allocation (for node lists) ───────────────────────────────────
    // Allocate a fixed-size array of pointers in the arena.

    template<typename T>
    T** allocArray(size_t n) {
        if (n == 0) return nullptr;
        return static_cast<T**>(
            arena_.allocate(n * sizeof(T*), alignof(T*))
        );
    }

    // ── Memory stats ─────────────────────────────────────────────────────────

    size_t bytesAllocated() const { return arena_.bytesAllocated(); }

private:
    ArenaAllocator arena_;
};

} // namespace vex