#pragma once
// include/vex/CodeGen/StringPool.h


// ============================================================================
// vex/CodeGen/StringPool.h  — LLVM-level string constant deduplication
// ============================================================================
#include "llvm/IR/Module.h"
#include "llvm/IR/GlobalVariable.h"
#include <unordered_map>
#include <string>
#include <string_view>

namespace vex {

class CodeGenStringPool {
public:
    explicit CodeGenStringPool(llvm::Module& mod) : mod_(mod) {}

    // Get or create a global constant string; returns i8*
    llvm::Constant* get(std::string_view s);

    // Total unique strings
    size_t size() const { return pool_.size(); }

private:
    llvm::Module& mod_;
    std::unordered_map<std::string, llvm::Constant*> pool_;
};
} // namespace vex
