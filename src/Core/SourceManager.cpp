// src/Core/SourceManager.cpp
#include "vex/Core/SourceManager.h"
#include "vex/Core/MemoryBuffer.h"
#include <algorithm>
#include <cassert>

namespace vex {

// ── FileEntry helpers ─────────────────────────────────────────────────────────

void FileEntry::buildLineOffsets() const {
    if (lineOffsetsBuilt) return;
    lineOffsetsBuilt = true;
    lineOffsets.clear();
    lineOffsets.push_back(0);
    if (!buffer) return;
    const char* data = buffer->data();
    size_t      size = buffer->size();
    for (size_t i = 0; i < size; ++i)
        if (data[i] == '\n')
            lineOffsets.push_back(static_cast<uint32_t>(i + 1));
}

// ── SourceManager ─────────────────────────────────────────────────────────────

FileID SourceManager::loadFile(std::string_view path) {
    auto it = pathToID_.find(std::string(path));
    if (it != pathToID_.end()) return it->second;
    auto buf = MemoryBuffer::fromFile(path);
    if (!buf) return FileID::invalid();
    return addBuffer(std::move(buf), path);
}

FileID SourceManager::addBuffer(std::unique_ptr<MemoryBuffer> buffer,
                                 std::string_view virtualPath) {
    FileID id{static_cast<uint32_t>(files_.size() + 1)};
    auto entry = std::make_unique<FileEntry>();
    entry->id     = id;
    entry->path   = std::string(virtualPath);
    entry->buffer = std::move(buffer);
    pathToID_[entry->path] = id;
    files_.push_back(std::move(entry));
    return id;
}

FileEntry* SourceManager::getEntry(FileID id) const {
    if (!id.isValid() || id.id > files_.size()) return nullptr;
    return files_[id.id - 1].get();
}

const MemoryBuffer* SourceManager::getBuffer(FileID id) const {
    auto* e = getEntry(id);
    return e ? e->buffer.get() : nullptr;
}

std::string_view SourceManager::getPath(FileID id) const {
    auto* e = getEntry(id);
    return e ? std::string_view(e->path) : std::string_view{};
}

SourceLocation SourceManager::getLocation(FileID id, uint32_t byteOffset) const {
    auto* e = getEntry(id);
    if (!e) return SourceLocation::invalid();
    e->buildLineOffsets();
    const auto& offs = e->lineOffsets;
    auto it = std::upper_bound(offs.begin(), offs.end(), byteOffset);
    uint32_t lineIdx = static_cast<uint32_t>(std::distance(offs.begin(), it));
    if (lineIdx == 0) lineIdx = 1;
    uint32_t lineStart = offs[lineIdx - 1];
    uint32_t col = byteOffset - lineStart + 1;
    return SourceLocation(id, lineIdx, col);
}

SourceLocation SourceManager::getLocation(FileID id, const char* ptr) const {
    const MemoryBuffer* buf = getBuffer(id);
    if (!buf || ptr < buf->data() || ptr > buf->data() + buf->size())
        return SourceLocation::invalid();
    return getLocation(id, static_cast<uint32_t>(ptr - buf->data()));
}

std::string_view SourceManager::getLineText(SourceLocation loc) const {
    auto* e = getEntry(loc.fileID());
    if (!e || !e->buffer) return {};
    e->buildLineOffsets();
    uint32_t lineIdx = loc.line();
    if (lineIdx == 0 || lineIdx > e->lineOffsets.size()) return {};
    const char* data = e->buffer->data();
    size_t      size = e->buffer->size();
    uint32_t start = e->lineOffsets[lineIdx - 1];
    uint32_t end   = (lineIdx < e->lineOffsets.size())
                   ? e->lineOffsets[lineIdx]
                   : static_cast<uint32_t>(size);
    while (end > start && (data[end-1] == '\n' || data[end-1] == '\r'))
        --end;
    return std::string_view(data + start, end - start);
}

std::string_view SourceManager::getFilePath(SourceLocation loc) const {
    return getPath(loc.fileID());
}

FileEntry* SourceManager::getOrCreateEntry(std::string path) {
    auto it = pathToID_.find(path);
    if (it != pathToID_.end()) return getEntry(it->second);
    FileID id = addBuffer(nullptr, path);
    return getEntry(id);
}

} // namespace vex
