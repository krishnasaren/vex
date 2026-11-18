#pragma once
#include <cstdint>
// include/vex/Driver/CompilationUnit.h


// ============================================================================
// vex/Driver/CompilationUnit.h  — A single source file being compiled
// ============================================================================
#include "vex/Core/SourceLocation.h"
#include "vex/AST/ASTContext.h"
#include "vex/IR/VexIR.h"
#include <string>
#include <memory>

namespace vex {
class ModuleDecl;

struct CompilationUnit {
    uint32_t          fileId;
    std::string       sourcePath;
    std::string       moduleName;

    // Populated progressively through the pipeline
    std::unique_ptr<ASTContext> astCtx;
    ModuleDecl*                  astRoot   = nullptr;
    std::unique_ptr<IRModule>    irModule;

    bool hasErrors = false;
};

} // namespace vex
