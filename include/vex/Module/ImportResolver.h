#pragma once
// include/vex/Module/ImportResolver.h





// ============================================================================
// vex/Module/ImportResolver.h  — Resolves import statements to file paths
// ============================================================================
#include "vex/AST/Decl.h"
#include "vex/Core/DiagnosticEngine.h"
#include "vex/Core/FileManager.h"
#include <string>
#include <string_view>
#include <vector>

namespace vex {

class ImportResolver {
public:
    ImportResolver(DiagnosticEngine& diags, FileManager& files)
        : diags_(diags), files_(files) {}

    void addSearchPath(std::string_view path);
    void addStdlibPath(std::string_view path);

    // Resolve an import path like "std::io" or "mylib::types"
    // Returns the .vex file path or .vxl precompiled path; empty on failure
    std::string resolve(std::string_view importPath,
                        std::string_view fromFile,
                        SourceLocation loc);

    // Check if a module has already been resolved (cached)
    bool isCached(std::string_view importPath) const;

private:
    DiagnosticEngine&        diags_;
    FileManager&             files_;
    std::vector<std::string> searchPaths_;
    std::vector<std::string> stdlibPaths_;
    std::unordered_map<std::string,std::string> cache_;

    std::string tryPath(std::string_view dir, std::string_view modPath);
    std::string importPathToFilePath(std::string_view importPath);
};
} // namespace vex
