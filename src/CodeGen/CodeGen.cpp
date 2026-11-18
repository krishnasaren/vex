// src/CodeGen/CodeGen.cpp — VexIR → LLVM IR → object/asm/bitcode
#include "vex/CodeGen/CodeGen.h"
#include "vex/CodeGen/LLVMEmitter.h"
#include "vex/Driver/Options.h"
#include "llvm/IR/LLVMContext.h"
#include "llvm/IR/Module.h"
#include "llvm/IR/Verifier.h"
#include "llvm/IR/PassManager.h"
#include "llvm/IR/LegacyPassManager.h"
#include "llvm/Analysis/CGSCCPassManager.h"
#include "llvm/Analysis/LoopAnalysisManager.h"
#include "llvm/Passes/PassBuilder.h"
#include "llvm/Support/FileSystem.h"
#include "llvm/Support/TargetSelect.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/Target/TargetMachine.h"
#include "llvm/Target/TargetOptions.h"
#include "llvm/TargetParser/Host.h"
#include "llvm/Bitcode/BitcodeWriter.h"
#include "llvm/MC/TargetRegistry.h"

namespace vex {

std::unique_ptr<llvm::Module> CodeGen::lower(IRModule& vexMod) {
    llvmCtx_ = std::make_unique<llvm::LLVMContext>();
    auto llvmMod = std::make_unique<llvm::Module>(vexMod.name(), *llvmCtx_);
    std::string triple = llvm::sys::getDefaultTargetTriple();
    if (!opts_.target.empty()) triple = opts_.target;
    llvmMod->setTargetTriple(triple);

    LLVMEmitter emitter(*llvmMod, diags_, opts_);
    if (!emitter.emit(vexMod)) return nullptr;

    // Optimize if requested
    if (opts_.optLevel > 0) runOptPipeline(*llvmMod, opts_.optLevel);

    if (llvm::verifyModule(*llvmMod, &llvm::errs())) return nullptr;
    return llvmMod;
}

void CodeGen::runOptPipeline(llvm::Module& mod, int level) {
    // Build an optimization pipeline using the new pass manager
    std::string triple = mod.getTargetTriple();
    if (triple.empty()) triple = llvm::sys::getDefaultTargetTriple();

    llvm::PassBuilder pb;
    llvm::LoopAnalysisManager       lam;
    llvm::FunctionAnalysisManager   fam;
    llvm::CGSCCAnalysisManager      cgam;
    llvm::ModuleAnalysisManager     mam;
    pb.registerModuleAnalyses(mam);
    pb.registerCGSCCAnalyses(cgam);
    pb.registerFunctionAnalyses(fam);
    pb.registerLoopAnalyses(lam);
    pb.crossRegisterProxies(lam, fam, cgam, mam);

    llvm::OptimizationLevel opt;
    switch (level) {
    case 1:  opt = llvm::OptimizationLevel::O1; break;
    case 2:  opt = llvm::OptimizationLevel::O2; break;
    case 3:  opt = llvm::OptimizationLevel::O3; break;
    default: opt = llvm::OptimizationLevel::O0; break;
    }
    llvm::ModulePassManager mpm = pb.buildPerModuleDefaultPipeline(opt);
    mpm.run(mod, mam);
}

static bool initTargets() {
    static bool done = false;
    if (!done) {
        llvm::InitializeAllTargetInfos();
        llvm::InitializeAllTargets();
        llvm::InitializeAllTargetMCs();
        llvm::InitializeAllAsmPrinters();
        llvm::InitializeAllAsmParsers();
        done = true;
    }
    return true;
}

static std::unique_ptr<llvm::TargetMachine>
createTM(const std::string& triple, int optLevel) {
    initTargets();
    std::string errMsg;
    const llvm::Target* tgt = llvm::TargetRegistry::lookupTarget(triple, errMsg);
    if (!tgt) {
        llvm::errs() << "vexc: target not found: " << errMsg << "\n";
        return nullptr;
    }
    llvm::TargetOptions topts;
    llvm::CodeGenOptLevel cgo;
    switch (optLevel) {
    case 0: cgo = llvm::CodeGenOptLevel::None;       break;
    case 1: cgo = llvm::CodeGenOptLevel::Less;       break;
    case 2: cgo = llvm::CodeGenOptLevel::Default;    break;
    default:cgo = llvm::CodeGenOptLevel::Aggressive; break;
    }
    std::unique_ptr<llvm::TargetMachine> tm(
        tgt->createTargetMachine(triple, "generic", "",
                                  topts, llvm::Reloc::PIC_, {}, cgo));
    return tm;
}

static void setDataLayout(llvm::Module& mod, llvm::TargetMachine* tm) {
    mod.setDataLayout(tm->createDataLayout());
    if (mod.getTargetTriple().empty())
        mod.setTargetTriple(tm->getTargetTriple().str());
}

bool CodeGen::emitObjectFile(llvm::Module& mod, std::string_view path) {
    auto tm = createTM(mod.getTargetTriple().empty()
        ? llvm::sys::getDefaultTargetTriple() : mod.getTargetTriple(),
        opts_.optLevel);
    if (!tm) return false;
    setDataLayout(mod, tm.get());

    std::error_code ec;
    llvm::raw_fd_ostream out(path, ec, llvm::sys::fs::OF_None);
    if (ec) { llvm::errs() << "vexc: " << ec.message() << "\n"; return false; }

    // LLVM 18: use legacy PM shim for code emission
    llvm::legacy::PassManager pm;
    if (tm->addPassesToEmitFile(pm, out, nullptr,
            llvm::CodeGenFileType::ObjectFile))
        return false;
    pm.run(mod);
    return !ec;
}

bool CodeGen::emitAssembly(llvm::Module& mod, std::string_view path) {
    auto tm = createTM(mod.getTargetTriple().empty()
        ? llvm::sys::getDefaultTargetTriple() : mod.getTargetTriple(),
        opts_.optLevel);
    if (!tm) return false;
    setDataLayout(mod, tm.get());

    std::error_code ec;
    llvm::raw_fd_ostream out(path, ec);
    if (ec) return false;

    llvm::legacy::PassManager pm;
    if (tm->addPassesToEmitFile(pm, out, nullptr,
            llvm::CodeGenFileType::AssemblyFile))
        return false;
    pm.run(mod);
    return !ec;
}

bool CodeGen::emitLLVMIR(llvm::Module& mod, std::string_view path) {
    std::error_code ec;
    llvm::raw_fd_ostream out(path, ec);
    if (ec) return false;
    mod.print(out, nullptr);
    return true;
}

bool CodeGen::emitBitcode(llvm::Module& mod, std::string_view path) {
    std::error_code ec;
    llvm::raw_fd_ostream out(path, ec, llvm::sys::fs::OF_None);
    if (ec) return false;
    llvm::WriteBitcodeToFile(mod, out);
    return true;
}

} // namespace vex
