// ============================================================================
// vex/Core/SourceManager.cpp
// ============================================================================

#include "vex/Core/SourceManager.h"
#include "vex/Core/Assert.h"
#include <algorithm>
#include <filesystem>

namespace vex {

// ── FileEntry ─────────────────────────────────────────────────────────────────

void FileEntry::buildLineOffsets() const {
    if (lineOffsetsBuilt) return;

    lineOffsets.clear();
    lineOffsets.push_back(0); // line 1 starts at offset 0

    if (!buffer) {
        lineOffsetsBuilt = true;
        return;
    }

    const char* data = buffer->data();
    size_t      sz   = buffer->size();

    for (size_t i = 0; i < sz; ++i) {
        if (data[i] == '\n') {
            if (i + 1 <= sz)
                lineOffsets.push_back(static_cast<uint32_t>(i + 1));
        }
    }
    lineOffsetsBuilt = true;
}

// ── SourceManager ─────────────────────────────────────────────────────────────

FileID SourceManager::loadFile(std::string_view path) {
    // Canonicalize path
    std::string absPath;
    try {
        absPath = std::filesystem::absolute(std::string(path)).string();
    } catch (...) {
        absPath = std::string(path);
    }

    // Return cached FileID if already loaded
    auto it = pathToID_.find(absPath);
    if (it != pathToID_.end())
        return it->second;

    auto buf = MemoryBuffer::fromFile(absPath);
    if (!buf)
        return FileID::invalid();

    return addBuffer(std::move(buf), absPath);
}

FileID SourceManager::addBuffer(
    std::unique_ptr<MemoryBuffer> buffer,
    std::string_view              virtualPath)
{
    FileID id{nextID_++};

    auto entry           = std::make_unique<FileEntry>();
    entry->id            = id;
    entry->path          = std::string(virtualPath);
    entry->buffer        = std::move(buffer);
    entry->lineOffsetsBuilt = false;

    pathToID_[entry->path] = id;
    files_.push_back(std::move(entry));
    return id;
}

const MemoryBuffer* SourceManager::getBuffer(FileID id) const {
    auto* entry = getEntry(id);
    return entry ? entry->buffer.get() : nullptr;
}

std::string_view SourceManager::getPath(FileID id) const {
    auto* entry = getEntry(id);
    return entry ? std::string_view(entry->path) : std::string_view{};
}

SourceLocation SourceManager::getLocation(FileID id, const char* ptr) const {
    auto* entry = getEntry(id);
    if (!entry || !entry->buffer) return SourceLocation::invalid();

    const char* base = entry->buffer->data();
    size_t      sz   = entry->buffer->size();

    if (ptr < base || ptr > base + sz)
        return SourceLocation::invalid();

    uint32_t offset = static_cast<uint32_t>(ptr - base);
    return getLocation(id, offset);
}

SourceLocation SourceManager::getLocation(FileID id, uint32_t byteOffset) const {
    auto* entry = getEntry(id);
    if (!entry) return SourceLocation::invalid();

    entry->buildLineOffsets();
    const auto& offsets = entry->lineOffsets;

    if (offsets.empty()) return SourceLocation(id, 1, byteOffset + 1);

    // Binary search: find the last line start <= byteOffset
    auto it = std::upper_bound(offsets.begin(), offsets.end(), byteOffset);
    uint32_t lineIdx  = static_cast<uint32_t>(std::distance(offsets.begin(), it)) - 1;
    uint32_t lineStart = offsets[lineIdx];
    uint32_t col       = byteOffset - lineStart + 1; // 1-based

    return SourceLocation(id, lineIdx + 1, col); // line is 1-based
}

std::string_view SourceManager::getLineText(SourceLocation loc) const {
    auto* entry = getEntry(loc.fileID());
    if (!entry || !entry->buffer) return {};

    entry->buildLineOffsets();
    const auto& offsets = entry->lineOffsets;

    uint32_t lineIdx = loc.line() - 1; // convert to 0-based
    if (lineIdx >= static_cast<uint32_t>(offsets.size())) return {};

    uint32_t start = offsets[lineIdx];
    uint32_t end;

    if (lineIdx + 1 < static_cast<uint32_t>(offsets.size()))
        end = offsets[lineIdx + 1];
    else
        end = static_cast<uint32_t>(entry->buffer->size());

    const char* data = entry->buffer->data();

    // Strip trailing newline chars
    while (end > start &&
           (data[end - 1] == '\n' || data[end - 1] == '\r'))
        --end;

    return {data + start, end - start};
}

std::string_view SourceManager::getFilePath(SourceLocation loc) const {
    return getPath(loc.fileID());
}

FileEntry* SourceManager::getEntry(FileID id) const {
    if (!id.isValid()) return nullptr;
    uint32_t idx = id.id - 1;
    if (idx >= files_.size()) return nullptr;
    return files_[idx].get();
}

} // namespace vex