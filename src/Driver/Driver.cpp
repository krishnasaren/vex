// src/Driver/Driver.cpp
// ============================================================================
// vex/Driver/Driver.cpp
// ============================================================================

#include "vex/Driver/Driver.h"
#include "vex/Core/MemoryBuffer.h"
#include "vex/Core/Version.h"
#include "vex/Lexer/Lexer.h"
#include "vex/Parser/Parser.h"
#include "vex/AST/ASTDumper.h"
#include "vex/IR/IRGen.h"
#include "vex/CodeGen/CodeGen.h"
#include "vex/CodeGen/LLVMEmitter.h"

#include <filesystem>
#include <cstdio>

namespace vex {

Driver::Driver(VexContext& ctx) : ctx_(ctx) {}

bool Driver::compile(const std::vector<std::string>& inputFiles) {
    bool ok = true;
    for (auto& path : inputFiles) {
        if (!compileFile(path)) ok = false;
        if (ctx_.shouldAbort()) break;
    }
    return ok;
}

bool Driver::compileFile(const std::string& path) {
    // ── Phase 1: Load ─────────────────────────────────────────────────────────
    auto buf = MemoryBuffer::fromFile(path);
    if (!buf) {
        fprintf(stderr, "vexc: cannot open file: %s\n", path.c_str());
        return false;
    }

    FileID fileID = ctx_.srcMgr().addBuffer(std::move(buf), path);
    const MemoryBuffer* mbuf = ctx_.srcMgr().getBuffer(fileID);

    // ── Phase 2: Lex ─────────────────────────────────────────────────────────
    Lexer lexer(*mbuf, fileID, ctx_.srcMgr(), ctx_.diags());
    auto tokens = lexer.lexAll();

    if (ctx_.hasErrors()) return false;

    // ── Phase 3: Parse ───────────────────────────────────────────────────────
    ASTContext astCtx;
    std::string moduleName = std::filesystem::path(path).stem().string();

    Parser parser(std::move(tokens), astCtx, ctx_.srcMgr(), ctx_.diags());
    ModuleDecl* mod = parser.parseModule(moduleName);

    if (ctx_.hasErrors() || !mod) return false;

    // ── Phase 4: IRGen ───────────────────────────────────────────────────────
    IRGen irgen(ctx_.diags());
    auto irMod = irgen.generate(mod);

    if (ctx_.hasErrors() || !irMod) return false;

    // ── Phase 5: LLVM emission ────────────────────────────────────────────────
    CodeGen cg(ctx_);
    LLVMEmitter emitter(ctx_, cg.llvmContext(), moduleName);
    auto llvmMod = emitter.emit(irMod.get());

    if (!llvmMod) return false;

    // ── Phase 6: Optimize ────────────────────────────────────────────────────
    cg.optimize(llvmMod.get());

    // ── Phase 7: Write output ─────────────────────────────────────────────────
    const auto& opts = ctx_.opts();
    std::string outPath = opts.outputPath;
    if (outPath.empty()) {
        outPath = moduleName;
#if defined(_WIN32)
        if (opts.outputKind == CompilerOptions::OutputKind::Executable) outPath += ".exe";
#endif
    }

    OutputFileKind kind;
    switch (opts.outputKind) {
        case CompilerOptions::OutputKind::LLVMAssembly: kind = OutputFileKind::LLVMAssembly; break;
        case CompilerOptions::OutputKind::LLVMBitcode:  kind = OutputFileKind::LLVMBitcode;  break;
        default:                                        kind = OutputFileKind::ObjectFile;    break;
    }

    if (!cg.writeOutput(llvmMod.get(), outPath + ".o", OutputFileKind::ObjectFile))
        return false;

    if (opts.outputKind == CompilerOptions::OutputKind::LLVMAssembly)
        cg.writeOutput(llvmMod.get(), outPath + ".ll", OutputFileKind::LLVMAssembly);

    fprintf(stdout, "vexc: compiled %s → %s.o\n", path.c_str(), outPath.c_str());
    return true;
}

bool Driver::checkFile(const std::string& path) {
    auto buf = MemoryBuffer::fromFile(path);
    if (!buf) {
        fprintf(stderr, "vexc: cannot open file: %s\n", path.c_str());
        return false;
    }

    FileID fileID = ctx_.srcMgr().addBuffer(std::move(buf), path);
    const MemoryBuffer* mbuf = ctx_.srcMgr().getBuffer(fileID);

    Lexer lexer(*mbuf, fileID, ctx_.srcMgr(), ctx_.diags());
    auto tokens = lexer.lexAll();
    if (ctx_.hasErrors()) return false;

    ASTContext astCtx;
    std::string moduleName = std::filesystem::path(path).stem().string();
    Parser parser(std::move(tokens), astCtx, ctx_.srcMgr(), ctx_.diags());
    ModuleDecl* mod = parser.parseModule(moduleName);

    if (ctx_.hasErrors() || !mod) return false;

    fprintf(stdout, "vexc: %s — OK\n", path.c_str());
    return true;
}

bool Driver::dumpAST(const std::string& path) {
    auto buf = MemoryBuffer::fromFile(path);
    if (!buf) {
        fprintf(stderr, "vexc: cannot open file: %s\n", path.c_str());
        return false;
    }

    FileID fileID = ctx_.srcMgr().addBuffer(std::move(buf), path);
    const MemoryBuffer* mbuf = ctx_.srcMgr().getBuffer(fileID);

    Lexer lexer(*mbuf, fileID, ctx_.srcMgr(), ctx_.diags());
    auto tokens = lexer.lexAll();
    if (ctx_.hasErrors()) return false;

    ASTContext astCtx;
    std::string moduleName = std::filesystem::path(path).stem().string();
    Parser parser(std::move(tokens), astCtx, ctx_.srcMgr(), ctx_.diags());
    ModuleDecl* mod = parser.parseModule(moduleName);
    if (ctx_.hasErrors() || !mod) return false;

    ASTDumper dumper(ctx_.srcMgr(), stdout, /*useColor=*/true);
    dumper.dump(mod);
    return true;
}

bool Driver::dumpIR(const std::string& path) {
    auto buf = MemoryBuffer::fromFile(path);
    if (!buf) {
        fprintf(stderr, "vexc: cannot open file: %s\n", path.c_str());
        return false;
    }

    FileID fileID = ctx_.srcMgr().addBuffer(std::move(buf), path);
    const MemoryBuffer* mbuf = ctx_.srcMgr().getBuffer(fileID);

    Lexer lexer(*mbuf, fileID, ctx_.srcMgr(), ctx_.diags());
    auto tokens = lexer.lexAll();
    if (ctx_.hasErrors()) return false;

    ASTContext astCtx;
    std::string moduleName = std::filesystem::path(path).stem().string();
    Parser parser(std::move(tokens), astCtx, ctx_.srcMgr(), ctx_.diags());
    ModuleDecl* mod = parser.parseModule(moduleName);
    if (ctx_.hasErrors() || !mod) return false;

    IRGen irgen(ctx_.diags());
    auto irMod = irgen.generate(mod);
    if (!irMod) return false;

    // Dump VexIR
    irMod->dump(llvm::outs());

    // Also emit LLVM IR
    CodeGen cg(ctx_);
    LLVMEmitter emitter(ctx_, cg.llvmContext(), moduleName);
    auto llvmMod = emitter.emit(irMod.get());
    if (llvmMod) {
        fprintf(stdout, "\n; ── LLVM IR ─────────────────────────────────\n");
        llvmMod->print(llvm::outs(), nullptr);
    }
    return true;
}

} // namespace vex