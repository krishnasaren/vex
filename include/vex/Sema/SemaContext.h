#pragma once
// include/vex/Sema/SemaContext.h




#pragma once
// ============================================================================
// vex/Sema/SemaContext.h
// Shared context for all semantic analysis passes.
// ============================================================================

#include "vex/Sema/SymbolTable.h"
#include "vex/Core/DiagnosticEngine.h"
#include "vex/Core/VexContext.h"
#include "vex/AST/ASTContext.h"
#include <string_view>

namespace vex {

class FnDecl;
class ClassDecl;
class TraitDecl;

class SemaContext {
public:
    explicit SemaContext(VexContext& ctx, ASTContext& astCtx)
        : vexCtx_(ctx), astCtx_(astCtx), diags_(ctx.diags()) {}

    VexContext&        vexCtx()  { return vexCtx_; }
    ASTContext&        astCtx()  { return astCtx_; }
    DiagnosticEngine&  diags()   { return diags_; }
    SymbolTable&       symbols() { return symbols_; }

    // Current enclosing function (for return-type checking, yield, await)
    FnDecl*  currentFn     = nullptr;
    // Current enclosing class (for self, prot access)
    ClassDecl* currentClass = nullptr;
    // Current trait being implemented
    TraitDecl* currentTrait = nullptr;

    // Nesting counters
    uint32_t loopDepth     = 0;
    uint32_t asyncDepth    = 0;
    uint32_t generatorDepth= 0;
    uint32_t unsafeDepth   = 0;

    bool inImport()     const { return inImport_; }
    void enterImport()        { inImport_ = true; }
    void leaveImport()        { inImport_ = false; }

    bool hasErrors() const { return diags_.hasErrors(); }

private:
    VexContext&       vexCtx_;
    ASTContext&       astCtx_;
    DiagnosticEngine& diags_;
    SymbolTable       symbols_;
    bool              inImport_ = false;
};

} // namespace vex
