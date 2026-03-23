#pragma once
// include/vex/CodeGen/LLVMContext.h

// ============================================================================
// vex/CodeGen/LLVMContext.h  — Wraps llvm::LLVMContext with VEX conveniences
// ============================================================================
#include "llvm/IR/LLVMContext.h"
#include "llvm/IR/Module.h"
#include <memory>
#include <string_view>

namespace vex {

class LLVMContextWrapper {
public:
    LLVMContextWrapper();
    ~LLVMContextWrapper();

    llvm::LLVMContext&                ctx()    { return *ctx_; }
    const llvm::LLVMContext&          ctx()    const { return *ctx_; }

    std::unique_ptr<llvm::Module> createModule(std::string_view name,
                                                std::string_view triple);

    // Disable copy
    LLVMContextWrapper(const LLVMContextWrapper&) = delete;
    LLVMContextWrapper& operator=(const LLVMContextWrapper&) = delete;

private:
    std::unique_ptr<llvm::LLVMContext> ctx_;
};

} // namespace vex


