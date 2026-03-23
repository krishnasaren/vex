#pragma once
// include/vex/Core/BumpAllocator.h


// ============================================================================
// vex/Core/BumpAllocator.h  — Fast arena (bump pointer) allocator
// All AST nodes are allocated from this arena; freeing is bulk (whole arena).
// ============================================================================
#include <cstddef>
#include <cstdint>
#include <vector>

namespace vex {

class BumpAllocator {
public:
    explicit BumpAllocator(size_t chunkSize = 64 * 1024);
    ~BumpAllocator();

    void* allocate(size_t bytes, size_t align = 8);

    template<typename T, typename... Args>
    T* make(Args&&... args) {
        void* p = allocate(sizeof(T), alignof(T));
        return new(p) T(std::forward<Args>(args)...);
    }

    void  reset();    // Frees all without calling dtors
    size_t bytesUsed() const { return bytesUsed_; }

private:
    struct Chunk {
        uint8_t* data;
        size_t   size;
        size_t   used;
    };

    size_t              chunkSize_;
    size_t              bytesUsed_ = 0;
    std::vector<Chunk>  chunks_;

    void newChunk(size_t minSize);
    Chunk* currentChunk();
};

} // namespace vex
