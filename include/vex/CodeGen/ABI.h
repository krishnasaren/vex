#pragma once
// include/vex/CodeGen/ABI.h


// ============================================================================
// vex/CodeGen/ABI.h  — Abstract ABI interface for argument passing / return
// ============================================================================
#include "llvm/IR/Function.h"
#include "llvm/IR/Type.h"
#include "vex/AST/Decl.h"
#include <vector>

namespace vex {

class LLVMTypeMapper;

// How a single value is passed according to ABI rules
enum class ABIArgKind : uint8_t {
    Direct,       // pass by value in register(s)
    Indirect,     // pass via hidden pointer (byval / sret)
    Expand,       // split into multiple arguments
    Ignore,       // zero-sized (unit type)
    CoerceInt,    // coerce to integer register
    CoerceVec,    // coerce to float/vector register
};

struct ABIArgInfo {
    ABIArgKind     kind     = ABIArgKind::Direct;
    llvm::Type*    coerceTo = nullptr;   // for Coerce variants
    bool           inReg    = false;     // put in register
    bool           isSigned = true;
    unsigned       align    = 0;
};

class ABIInfo {
public:
    virtual ~ABIInfo() = default;

    virtual ABIArgInfo classifyReturnType(llvm::Type* ty) const = 0;
    virtual ABIArgInfo classifyArgType(llvm::Type* ty)   const = 0;

    // Rewrite an LLVM function type to match ABI rules
    virtual llvm::FunctionType* applyABI(llvm::FunctionType* ft,
                                          std::vector<ABIArgInfo>& argInfos) const = 0;
};

// ── Concrete ABIs ─────────────────────────────────────────────────────────────
class SysVx86_64ABI : public ABIInfo {
public:
    ABIArgInfo classifyReturnType(llvm::Type* ty) const override;
    ABIArgInfo classifyArgType(llvm::Type* ty)   const override;
    llvm::FunctionType* applyABI(llvm::FunctionType* ft,
                                  std::vector<ABIArgInfo>& infos) const override;
};

class Win64ABI : public ABIInfo {
public:
    ABIArgInfo classifyReturnType(llvm::Type* ty) const override;
    ABIArgInfo classifyArgType(llvm::Type* ty)   const override;
    llvm::FunctionType* applyABI(llvm::FunctionType* ft,
                                  std::vector<ABIArgInfo>& infos) const override;
};

class AAPCS64ABI : public ABIInfo {
public:
    ABIArgInfo classifyReturnType(llvm::Type* ty) const override;
    ABIArgInfo classifyArgType(llvm::Type* ty)   const override;
    llvm::FunctionType* applyABI(llvm::FunctionType* ft,
                                  std::vector<ABIArgInfo>& infos) const override;
};

class WasmABI : public ABIInfo {
public:
    ABIArgInfo classifyReturnType(llvm::Type* ty) const override;
    ABIArgInfo classifyArgType(llvm::Type* ty)   const override;
    llvm::FunctionType* applyABI(llvm::FunctionType* ft,
                                  std::vector<ABIArgInfo>& infos) const override;
};

// Factory
std::unique_ptr<ABIInfo> createABIForTriple(std::string_view triple);

} // namespace vex
