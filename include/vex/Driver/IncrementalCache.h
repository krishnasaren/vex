#pragma once
// include/vex/Driver/IncrementalCache.h

// ============================================================================
// vex/Driver/IncrementalCache.h  — Content-hash based incremental build cache
// ============================================================================
#include <string>
#include <string_view>
#include <unordered_map>

namespace vex {

class IncrementalCache {
public:
    explicit IncrementalCache(std::string_view cacheDir);

    // Returns true if cache hit — the output file is already up to date
    bool isUpToDate(std::string_view sourceHash,
                    std::string_view outputPath);

    // Record a compilation result
    void record(std::string_view sourceHash,
                std::string_view outputPath);

    // Hash a source file's contents + options key
    static std::string hashFile(std::string_view path, std::string_view optsKey);

    void flush();

private:
    std::string cacheDir_;
    std::string cacheFile_;
    std::unordered_map<std::string,std::string> entries_; // hash → outputPath
    bool dirty_ = false;

    void load();
    void save();
};

} // namespace vex
