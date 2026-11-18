// src/Driver/Driver.cpp — Main compilation pipeline
#include "vex/Driver/Driver.h"
#include "vex/AST/ASTContext.h"
#include "vex/IR/VexIR.h"
#include "vex/Core/MemoryBuffer.h"
#include "vex/Lexer/Lexer.h"
#include "vex/Parser/Parser.h"
#include "vex/Sema/Sema.h"
#include "vex/AST/ASTDumper.h"
#include "vex/IR/IRGen.h"
#include "vex/CodeGen/CodeGen.h"
#include "llvm/Support/raw_ostream.h"
#include <filesystem>
#include <iostream>
#include <cstdio>

namespace vex {

Driver::Driver(VexContext& ctx) : ctx_(ctx) {}

bool Driver::compile(const std::vector<std::string>& inputFiles) {
    bool ok = true;
    for (const auto& path : inputFiles) {
        if (!compileFile(path)) ok = false;
        if (ctx_.diags().shouldAbort()) break;
    }
    return ok;
}

static ModuleDecl* parseFile(VexContext& vctx, const std::string& path,
                              ASTContext& astCtx, std::string& moduleName) {
    auto buf = MemoryBuffer::fromFile(path);
    if (!buf) { fprintf(stderr, "vexc: cannot open '%s'\n", path.c_str()); return nullptr; }
    FileID fid = vctx.srcMgr().addBuffer(std::move(buf), path);
    const MemoryBuffer* mbuf = vctx.srcMgr().getBuffer(fid);
    if (!mbuf) return nullptr;
    Lexer lexer(*mbuf, fid, vctx.srcMgr(), vctx.diags());
    auto tokens = lexer.lexAll();
    if (vctx.diags().hasErrors()) return nullptr;
    moduleName = std::filesystem::path(path).stem().string();
    Parser parser(std::move(tokens), astCtx, vctx.srcMgr(), vctx.diags());
    ModuleDecl* mod = parser.parseModule(moduleName);
    if (vctx.diags().hasErrors() || !mod) return nullptr;
    return mod;
}

// Map CompilerOptions → DriverOptions for CodeGen
static DriverOptions toDriverOptions(const CompilerOptions& co) {
    DriverOptions d;
    d.target   = co.targetString();
    switch (co.optLevel) {
    case CompilerOptions::OptLevel::O0: d.optLevel = 0; break;
    case CompilerOptions::OptLevel::O1: d.optLevel = 1; break;
    case CompilerOptions::OptLevel::O2: d.optLevel = 2; break;
    case CompilerOptions::OptLevel::O3: d.optLevel = 3; break;
    case CompilerOptions::OptLevel::Os: d.optLevel = 2; break;
    }
    d.emitDebugInfo = co.emitDebugInfo;
    return d;
}

bool Driver::compileFile(const std::string& path) {
    ASTContext astCtx;
    std::string moduleName;
    ModuleDecl* mod = parseFile(ctx_, path, astCtx, moduleName);
    if (!mod) return false;

    // Semantic analysis
    Sema sema(ctx_, astCtx);
    if (!sema.run(mod) && ctx_.diags().hasErrors()) return false;

    // IRGen
    IRGen irgen(ctx_.diags());
    auto irMod = irgen.generate(mod);
    if (!irMod) return false;

    // Lower to LLVM
    DriverOptions dopts = toDriverOptions(ctx_.opts());
    dopts.outputFile = ctx_.opts().outputPath.empty()
                       ? moduleName : ctx_.opts().outputPath;
    CodeGen cg(ctx_.diags(), dopts);
    auto llvmMod = cg.lower(*irMod);
    if (!llvmMod) return false;

    std::string outBase = dopts.outputFile;

    using OK = CompilerOptions::OutputKind;
    switch (ctx_.opts().outputKind) {
    case OK::LLVMAssembly:
        return cg.emitLLVMIR(*llvmMod, outBase + ".ll");
    case OK::LLVMBitcode:
        return cg.emitBitcode(*llvmMod, outBase + ".bc");
    case OK::ObjectFile:
        return cg.emitObjectFile(*llvmMod, outBase + ".o");
    case OK::VxlLibrary:
        irMod->dump(std::cout);
        return true;
    default:
        // Executable or SharedLib: compile to .o
        return cg.emitObjectFile(*llvmMod, outBase + ".o");
    }
}

bool Driver::checkFile(const std::string& path) {
    ASTContext astCtx;
    std::string moduleName;
    ModuleDecl* mod = parseFile(ctx_, path, astCtx, moduleName);
    if (!mod) return false;
    Sema sema(ctx_, astCtx);
    sema.run(mod);
    bool ok = !ctx_.diags().hasErrors();
    fprintf(stdout, "vexc: %s — %s\n", path.c_str(), ok ? "OK" : "FAILED");
    return ok;
}

bool Driver::dumpAST(const std::string& path) {
    ASTContext astCtx;
    std::string moduleName;
    ModuleDecl* mod = parseFile(ctx_, path, astCtx, moduleName);
    if (!mod) return false;
    ASTDumper dumper(ctx_.srcMgr(), stdout, /*color=*/true);
    dumper.dump(mod);
    return true;
}

bool Driver::dumpIR(const std::string& path) {
    ASTContext astCtx;
    std::string moduleName;
    ModuleDecl* mod = parseFile(ctx_, path, astCtx, moduleName);
    if (!mod) return false;
    IRGen irgen(ctx_.diags());
    auto irMod = irgen.generate(mod);
    if (!irMod) return false;
    irMod->dump(std::cout);
    DriverOptions dopts = toDriverOptions(ctx_.opts());
    CodeGen cg(ctx_.diags(), dopts);
    auto llvmMod = cg.lower(*irMod);
    if (llvmMod) {
        fprintf(stdout, "\n; ── LLVM IR ──────────────────────────────────\n");
        llvmMod->print(llvm::outs(), nullptr);
    }
    return true;
}

} // namespace vex
