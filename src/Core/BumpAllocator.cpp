// src/Core/BumpAllocator.cpp
#include "vex/Core/BumpAllocator.h"
#include <cstdlib>
#include <cstring>
#include <algorithm>
#include <stdexcept>

namespace vex {

BumpAllocator::BumpAllocator(size_t chunkSize)
    : chunkSize_(chunkSize) {
    newChunk(chunkSize);
}

BumpAllocator::~BumpAllocator() {
    for (auto& c : chunks_)
        std::free(c.data);
}

void* BumpAllocator::allocate(size_t bytes, size_t align) {
    if (bytes == 0) return nullptr;

    Chunk* c = currentChunk();
    // Align the current write position
    size_t offset = c->used;
    size_t remainder = offset % align;
    if (remainder != 0) offset += align - remainder;

    if (offset + bytes > c->size) {
        // Need a new chunk
        newChunk(std::max(chunkSize_, bytes + align));
        c      = currentChunk();
        offset = 0;
    }

    void* ptr = c->data + offset;
    c->used   = offset + bytes;
    bytesUsed_ += bytes;
    return ptr;
}

void BumpAllocator::reset() {
    for (auto& c : chunks_) {
        std::free(c.data);
    }
    chunks_.clear();
    bytesUsed_ = 0;
    newChunk(chunkSize_);
}

void BumpAllocator::newChunk(size_t minSize) {
    size_t size = std::max(chunkSize_, minSize);
    uint8_t* data = static_cast<uint8_t*>(std::malloc(size));
    if (!data) throw std::bad_alloc();
    chunks_.push_back({data, size, 0});
}

BumpAllocator::Chunk* BumpAllocator::currentChunk() {
    return &chunks_.back();
}

} // namespace vex
