#pragma once
// include/vex/Linker/ExportTable.h


#pragma once
// vex/Linker/ExportTable.h  — Tracks exported symbols for shared libs / Wasm
#include <string>
#include <vector>
#include <unordered_set>
namespace vex {
class ExportTable {
public:
    void   addExport(std::string_view sym) { exports_.insert(std::string(sym)); }
    bool   isExported(std::string_view sym) const { return exports_.count(std::string(sym)); }
    void   removeExport(std::string_view sym) { exports_.erase(std::string(sym)); }
    const  std::unordered_set<std::string>& all() const { return exports_; }
    size_t size() const { return exports_.size(); }

    // Write an export list file (for --export-dynamic-symbol-list)
    bool writeExportList(std::string_view path) const;

private:
    std::unordered_set<std::string> exports_;
};
} // namespace vex
