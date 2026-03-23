#pragma once
// include/vex/CodeGen/Monomorphizer.h


// ============================================================================
// vex/CodeGen/Monomorphizer.h  — Instantiates generic functions/types
// ============================================================================
#include "vex/AST/Decl.h"
#include "vex/AST/Type.h"
#include "vex/Sema/SemaContext.h"
#include <vector>
#include <unordered_map>
#include <string>

namespace vex {

// A monomorphization key: generic name + type args
struct MonoKey {
    std::string             baseName;
    std::vector<TypeNode*>  typeArgs;

    bool operator==(const MonoKey& o) const;
    std::string mangledName() const;
};

class Monomorphizer {
public:
    explicit Monomorphizer(SemaContext& ctx) : ctx_(ctx) {}

    // Request a monomorphization; returns the concrete Decl
    FnDecl*     instantiateFn(FnDecl* tmpl,
                               const std::vector<TypeNode*>& typeArgs);
    StructDecl* instantiateStruct(StructDecl* tmpl,
                                   const std::vector<TypeNode*>& typeArgs);

    // Apply type substitution map to a type
    TypeNode* substitute(TypeNode* ty,
                          const std::unordered_map<std::string,TypeNode*>& subs);

    // All instantiated functions (to be code-generated)
    const std::vector<FnDecl*>&     instantiatedFns()     const { return instFns_; }
    const std::vector<StructDecl*>& instantiatedStructs() const { return instStructs_; }

private:
    SemaContext&                ctx_;
    std::vector<FnDecl*>        instFns_;
    std::vector<StructDecl*>    instStructs_;
    std::unordered_map<std::string, FnDecl*>     fnCache_;
    std::unordered_map<std::string, StructDecl*> structCache_;
};

} // namespace vex
