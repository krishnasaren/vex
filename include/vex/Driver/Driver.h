#pragma once
// include/vex/Driver/Driver.h

#pragma once
// ============================================================================
// vex/Driver/Driver.h
// Orchestrates the full compilation pipeline:
//   Source → Lex → Parse → Sema → IRGen → LLVM → Link
// ============================================================================

#include "vex/Core/VexContext.h"
#include "vex/AST/ASTContext.h"
#include <string>
#include <vector>

namespace vex {

class Driver {
public:
    explicit Driver(VexContext& ctx);

    // Compile a list of source files to the configured output
    bool compile(const std::vector<std::string>& inputFiles);

    // Compile a single file; returns false on error
    bool compileFile(const std::string& path);

    // Perform parse+sema only (--check mode)
    bool checkFile(const std::string& path);

    // Dump AST to stdout (--dump-ast)
    bool dumpAST(const std::string& path);

    // Dump LLVM IR to stdout (--dump-ir)
    bool dumpIR(const std::string& path);

private:
    VexContext& ctx_;
};

} // namespace vex