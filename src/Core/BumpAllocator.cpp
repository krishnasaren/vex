// src/Core/BumpAllocator.cpp



#include "vex/Core/BumpAllocator.h"
#include <cstdlib>
#include <cassert>
#include <cstring>
#include <stdexcept>

namespace vex {

BumpAllocator::BumpAllocator(size_t chunkSize) : chunkSize_(chunkSize) {
    newChunk(chunkSize);
}

BumpAllocator::~BumpAllocator() {
    for (auto& c : chunks_)
        free(c.data);
}

void* BumpAllocator::allocate(size_t bytes, size_t align) {
    if (bytes == 0) return nullptr;
    Chunk* cur = currentChunk();
    // align up
    size_t aligned = (cur->used + (align - 1)) & ~(align - 1);
    if (aligned + bytes > cur->size)
        newChunk(std::max(chunkSize_, bytes + align));
    cur = currentChunk();
    aligned = (cur->used + (align - 1)) & ~(align - 1);
    void* p = cur->data + aligned;
    cur->used = aligned + bytes;
    bytesUsed_ += bytes;
    return p;
}

void BumpAllocator::reset() {
    for (auto& c : chunks_)
        c.used = 0;
    bytesUsed_ = 0;
}

void BumpAllocator::newChunk(size_t minSize) {
    Chunk c;
    c.size = std::max(chunkSize_, minSize);
    c.data = static_cast<uint8_t*>(malloc(c.size));
    if (!c.data) throw std::bad_alloc{};
    c.used = 0;
    chunks_.push_back(c);
}

BumpAllocator::Chunk* BumpAllocator::currentChunk() {
    return chunks_.empty() ? nullptr : &chunks_.back();
}

} // namespace vex



