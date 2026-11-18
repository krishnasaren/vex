#pragma once
// include/vex/Core/FileManager.h



// ============================================================================
// vex/Core/FileManager.h  — Manages source file loading and caching
// ============================================================================
#include "vex/Core/MemoryBuffer.h"
#include "vex/Core/Error.h"
#include <string>
#include <string_view>
#include <unordered_map>
#include <vector>
#include <memory>

namespace vex {

struct FileManagerEntry {
    uint32_t    id;
    std::string path;
    std::unique_ptr<MemoryBuffer> buffer;
};

class FileManager {
public:
    FileManager() = default;

    // Load a file from disk; returns null on error
    const FileManagerEntry* open(std::string_view path);

    // Register a virtual file (e.g. from a string in tests)
    const FileManagerEntry* openVirtual(std::string_view name, std::string contents);

    const FileManagerEntry* getById(uint32_t id) const;
    const FileManagerEntry* getByPath(std::string_view path) const;

    // All loaded files
    const std::vector<std::unique_ptr<FileManagerEntry>>& files() const { return files_; }

private:
    std::vector<std::unique_ptr<FileManagerEntry>>          files_;
    std::unordered_map<std::string, FileManagerEntry*>      byPath_;
    std::unordered_map<uint32_t,    FileManagerEntry*>      byId_;
    uint32_t                                          nextId_ = 1;
};

} // namespace vex
