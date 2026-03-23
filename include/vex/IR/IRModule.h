#pragma once
// include/vex/IR/IRModule.h


// ============================================================================
// vex/IR/IRModule.h  — Top-level VEX IR container (one per source module)
// ============================================================================
#include "vex/IR/IRFunction.h"
#include <vector>
#include <memory>
#include <string>
#include <unordered_map>

namespace vex {

class IRGlobal : public IRValue {
public:
    IRGlobal(std::string name, IRType* ty, IRValue* init = nullptr)
        : IRValue(IRValueKind::GlobalVariable, ty), init_(init) {
        setName(std::move(name));
    }
    IRValue*    initializer() const { return init_; }
    bool        isConst()     const { return const_; }
    void        setConst(bool v)    { const_ = v; }
    std::string_view linkage() const { return linkage_; }
    void        setLinkage(std::string l) { linkage_ = std::move(l); }

private:
    IRValue*    init_    = nullptr;
    bool        const_   = false;
    std::string linkage_ = "internal";
};

class IRModule {
public:
    explicit IRModule(std::string name, std::string triple)
        : name_(std::move(name)), triple_(std::move(triple)) {}

    std::string_view name()   const { return name_; }
    std::string_view triple() const { return triple_; }

    IRFunction* addFunction(std::string name, IRType* fnType);
    IRGlobal*   addGlobal(std::string name, IRType* ty, IRValue* init = nullptr);

    IRFunction* getFunction(std::string_view name) const;
    IRGlobal*   getGlobal(std::string_view name)   const;

    const std::vector<std::unique_ptr<IRFunction>>& functions() const { return functions_; }
    const std::vector<std::unique_ptr<IRGlobal>>&   globals()   const { return globals_; }

    void dump() const;

private:
    std::string                                name_;
    std::string                                triple_;
    std::vector<std::unique_ptr<IRFunction>>   functions_;
    std::vector<std::unique_ptr<IRGlobal>>     globals_;
    std::unordered_map<std::string,IRFunction*> fnMap_;
    std::unordered_map<std::string,IRGlobal*>   glMap_;
};

} // namespace vex
