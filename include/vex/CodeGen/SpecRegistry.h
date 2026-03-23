#pragma once
// include/vex/CodeGen/SpecRegistry.h




// ============================================================================
// vex/CodeGen/SpecRegistry.h  — Registry of all monomorphized specializations
// ============================================================================
#include "vex/AST/Decl.h"
#include <unordered_map>
#include <string>
#include <vector>

namespace vex {

class SpecRegistry {
public:
    void    registerFn(std::string mangledName, FnDecl* decl);
    FnDecl* lookupFn(std::string_view mangledName) const;

    void        registerStruct(std::string mangledName, StructDecl* decl);
    StructDecl* lookupStruct(std::string_view mangledName) const;

    bool hasFn(std::string_view name)     const { return fnMap_.count(std::string(name)); }
    bool hasStruct(std::string_view name) const { return stMap_.count(std::string(name)); }

    const std::unordered_map<std::string,FnDecl*>&     allFns()     const { return fnMap_; }
    const std::unordered_map<std::string,StructDecl*>& allStructs() const { return stMap_; }

private:
    std::unordered_map<std::string,FnDecl*>     fnMap_;
    std::unordered_map<std::string,StructDecl*> stMap_;
};
} // namespace vex
