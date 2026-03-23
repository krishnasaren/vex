#pragma once
// include/vex/Module/ModuleLoader.h


// ============================================================================
// vex/Module/ModuleLoader.h  — Loads, parses, and caches modules
// ============================================================================
#include "vex/Module/ModuleCache.h"
#include "vex/Module/ImportResolver.h"
#include "vex/Core/DiagnosticEngine.h"
#include "vex/Core/FileManager.h"
#include "vex/AST/ASTContext.h"
#include <memory>

namespace vex {

class ModuleLoader {
public:
    ModuleLoader(DiagnosticEngine& diags, FileManager& files,
                  ImportResolver& resolver)
        : diags_(diags), files_(files), resolver_(resolver) {}

    // Load a module by import path; returns nullptr on error
    ModuleDecl* load(std::string_view importPath,
                     std::string_view fromFile,
                     SourceLocation loc);

    // Load a module from an explicit file path
    ModuleDecl* loadFile(std::string_view filePath);

    ModuleCache& cache() { return cache_; }

private:
    DiagnosticEngine& diags_;
    FileManager&      files_;
    ImportResolver&   resolver_;
    ModuleCache       cache_;

    ModuleDecl* parseVexSource(const std::string& path);
    ModuleDecl* loadVxl(const std::string& path);
};
} // namespace vex
