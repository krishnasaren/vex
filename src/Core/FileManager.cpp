// src/Core/FileManager.cpp



#include "vex/Core/FileManager.h"
#include <fstream>
#include <sstream>

namespace vex {

const FileEntry* FileManager::open(std::string_view path) {
    std::string p(path);
    auto it = byPath_.find(p);
    if (it != byPath_.end()) return it->second;

    std::ifstream f(p, std::ios::binary);
    if (!f) return nullptr;

    std::ostringstream ss;
    ss << f.rdbuf();
    std::string content = ss.str();

    auto entry = std::make_unique<FileEntry>();
    entry->id     = nextId_++;
    entry->path   = p;
    entry->buffer = MemoryBuffer::fromString(std::move(content), p);

    FileEntry* raw = entry.get();
    byPath_[p]         = raw;
    byId_[raw->id]     = raw;
    files_.push_back(std::move(entry));
    return raw;
}

const FileEntry* FileManager::openVirtual(std::string_view name, std::string contents) {
    std::string n(name);
    auto entry = std::make_unique<FileEntry>();
    entry->id     = nextId_++;
    entry->path   = n;
    entry->buffer = MemoryBuffer::fromString(std::move(contents), n);

    FileEntry* raw = entry.get();
    byPath_[n]         = raw;
    byId_[raw->id]     = raw;
    files_.push_back(std::move(entry));
    return raw;
}

const FileEntry* FileManager::getById(uint32_t id) const {
    auto it = byId_.find(id); return it != byId_.end() ? it->second : nullptr;
}

const FileEntry* FileManager::getByPath(std::string_view path) const {
    auto it = byPath_.find(std::string(path)); return it != byPath_.end() ? it->second : nullptr;
}

} // namespace vex
