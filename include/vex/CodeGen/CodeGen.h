#pragma once
// include/vex/CodeGen/CodeGen.h


/*
// ============================================================================
// vex/CodeGen/CodeGen.h
// Main code generation pipeline: VexIR → LLVM IR → machine code / object file.
// ============================================================================

#include "vex/IR/VexIR.h"
#include "vex/Core/VexContext.h"

#include <llvm/IR/LLVMContext.h>
#include <llvm/Target/TargetMachine.h>
#include <memory>
#include <string>

namespace vex {

enum class OutputFileKind {
    LLVMAssembly,   // .ll  (human-readable IR)
    LLVMBitcode,    // .bc
    ObjectFile,     // .o
    Assembly,       // .s  (native asm)
};

class CodeGen {
public:
    explicit CodeGen(VexContext& ctx);

    // Run optimizations on the LLVM module
    void optimize(llvm::Module* mod);

    // Write output file
    bool writeOutput(llvm::Module*   mod,
                     const std::string& outputPath,
                     OutputFileKind  kind);

    llvm::LLVMContext& llvmContext() { return *llvmCtx_; }

private:
    VexContext&                        ctx_;
    std::unique_ptr<llvm::LLVMContext> llvmCtx_;
    std::unique_ptr<llvm::TargetMachine> targetMachine_;

    bool initTargetMachine();
    void runPassPipeline(llvm::Module* mod, unsigned optLevel);
};

} // namespace vex

*/
// ============================================================================
// vex/CodeGen/CodeGen.h  — Top-level code generation driver
// ============================================================================
#include "vex/IR/IRModule.h"
#include "vex/Driver/Options.h"
#include "vex/Core/DiagnosticEngine.h"
#include "llvm/IR/Module.h"
#include <memory>
#include <string_view>

namespace vex {

class CodeGen {
public:
    CodeGen(DiagnosticEngine& diags, const DriverOptions& opts)
        : diags_(diags), opts_(opts) {}

    // Lower a VEX IR module to an LLVM module
    std::unique_ptr<llvm::Module> lower(IRModule& vexMod);

    // Emit object file from LLVM module
    bool emitObjectFile(llvm::Module& mod, std::string_view path);

    // Emit LLVM IR text
    bool emitLLVMIR(llvm::Module& mod, std::string_view path);

    // Emit bitcode
    bool emitBitcode(llvm::Module& mod, std::string_view path);

    // Emit assembly
    bool emitAssembly(llvm::Module& mod, std::string_view path);

private:
    DiagnosticEngine& diags_;
    const DriverOptions& opts_;
};

} // namespace vex




