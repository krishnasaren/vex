#pragma once
// include/vex/Sema/DecoratorProcessor.h


// ============================================================================
// vex/Sema/DecoratorProcessor.h  — Processes all @decorator annotations
// ============================================================================
#include "vex/Sema/SemaContext.h"
#include "vex/AST/Decl.h"

namespace vex {

class DecoratorProcessor {
public:
    explicit DecoratorProcessor(SemaContext& ctx) : ctx_(ctx) {}

    bool run(ModuleDecl* mod);

    // Process all decorators on a single declaration
    bool processDecl(Decl* decl);

private:
    SemaContext& ctx_;

    bool processInline(FnDecl* fn);
    bool processNoInline(FnDecl* fn);
    bool processExtern(FnDecl* fn);
    bool processTest(FnDecl* fn);
    bool processDeprecated(Decl* decl);
    bool processCold(FnDecl* fn);
    bool processHot(FnDecl* fn);
    bool processNoReturn(FnDecl* fn);
    bool processUnsafe(FnDecl* fn);
    bool processExport(Decl* decl);
    bool processLink(Decl* decl);
    bool processRepr(StructDecl* s, std::string_view arg);
};

} // namespace vex
