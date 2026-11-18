// src/CodeGen/StringPool.cpp
#include "vex/CodeGen/StringPool.h"
#include "llvm/IR/Constants.h"
#include "llvm/IR/GlobalVariable.h"
#include "llvm/IR/Type.h"

namespace vex {

llvm::Constant* CodeGenStringPool::get(std::string_view s) {
    std::string key(s);
    auto it = pool_.find(key);
    if (it != pool_.end()) return it->second;

    llvm::LLVMContext& ctx = mod_.getContext();
    // Create null-terminated array
    auto* str_const = llvm::ConstantDataArray::getString(ctx, llvm::StringRef(s.data(), s.size()), /*AddNull=*/true);
    auto* gv = new llvm::GlobalVariable(
        mod_, str_const->getType(),
        /*isConstant=*/true,
        llvm::GlobalValue::PrivateLinkage, str_const, ".str");
    gv->setUnnamedAddr(llvm::GlobalValue::UnnamedAddr::Global);
    gv->setAlignment(llvm::MaybeAlign(1));

    // Return GEP to i8*
    llvm::Constant* zero = llvm::ConstantInt::get(llvm::Type::getInt32Ty(ctx), 0);
    llvm::Constant* indices[] = {zero, zero};
    llvm::Constant* ptr = llvm::ConstantExpr::getGetElementPtr(
        str_const->getType(), gv, indices);

    pool_[key] = ptr;
    return ptr;
}

} // namespace vex
