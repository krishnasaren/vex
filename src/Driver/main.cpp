// vex/Driver/main.cpp — VEX Compiler entry point
#include "vex/Core/VexContext.h"
#include "vex/Core/DiagnosticConsumer.h"
#include "vex/Core/MemoryBuffer.h"
#include "vex/Core/Version.h"
#include "vex/AST/ASTContext.h"
#include "vex/AST/ASTDumper.h"
#include "vex/Lexer/Lexer.h"
#include "vex/Parser/Parser.h"
#include "vex/Sema/Sema.h"
#include "vex/IR/IRGen.h"
#include "vex/IR/VexIR.h"
#include "vex/CodeGen/CodeGen.h"
#include "vex/Driver/Options.h"
#include "llvm/Support/raw_ostream.h"
#include <cstdio>
#include <iostream>
#include <unistd.h>
#include <cstring>
#include <string>
#include <vector>
#include <filesystem>

using namespace vex;

// ── Version / help ────────────────────────────────────────────────────────────

static void printVersion() {
    fprintf(stdout,
        "VEX Compiler v%s (language v%s)\n"
        "LLVM backend v%s\n",
        std::string(VEX_VERSION_STRING).c_str(),
        std::string(VEX_LANGUAGE_VERSION).c_str(),
        std::string(VEX_TARGET_LLVM_VERSION).c_str());
}

static void printHelp(const char* prog) {
    fprintf(stderr,
        "VEX Compiler v%s\n"
        "Usage: %s [options] <file.vex> ...\n\n"
        "Options:\n"
        "  -o <file>        Output file\n"
        "  -O0/-O1/-O2/-O3  Optimisation level (default: -O0)\n"
        "  -Os              Optimise for size\n"
        "  -g               Emit debug information\n"
        "  --target <t>     Target triple  (default: host)\n"
        "  -c               Compile to object file only (.o)\n"
        "  -shared          Build as shared library\n"
        "  --emit-llvm      Emit LLVM IR (.ll)\n"
        "  -S               Emit assembly (.s)\n"
        "  --check          Type-check only, no codegen\n"
        "  --dump-ast       Print AST to stdout\n"
        "  --dump-ir        Print VEX IR and LLVM IR\n"
        "  -Werror          Warnings as errors\n"
        "  --version        Show compiler version\n"
        "  -h --help        Show this help\n\n"
        "Examples:\n"
        "  vexc hello.vex              → hello (executable)\n"
        "  vexc hello.vex -o greet     → greet\n"
        "  vexc --emit-llvm hello.vex  → hello.ll\n"
        "  vexc --check    hello.vex   → type-check only\n",
        std::string(VEX_VERSION_STRING).c_str(), prog);
}

// ── CLI ───────────────────────────────────────────────────────────────────────

struct Job {
    DriverOptions             opts;
    CompilerOptions           cOpts;
    std::vector<std::string>  inputFiles;
};

static bool parseCLI(int argc, char** argv, Job& job) {
    for (int i = 1; i < argc; ++i) {
        std::string_view arg = argv[i];
        if (arg == "--version")       { printVersion(); return false; }
        if (arg == "-h" || arg=="--help"){ printHelp(argv[0]); return false; }
        if (arg == "-O0") { job.opts.optLevel = 0; continue; }
        if (arg == "-O1") { job.opts.optLevel = 1; continue; }
        if (arg == "-O2") { job.opts.optLevel = 2; continue; }
        if (arg == "-O3") { job.opts.optLevel = 3; continue; }
        if (arg == "-Os" || arg == "-Oz") { job.opts.optLevel = 2; continue; }
        if (arg == "-g")  { job.opts.emitDebugInfo = true; continue; }
        if (arg == "-c")  { job.opts.outputKind = OutputKind::ObjectFile; continue; }
        if (arg == "-shared") { job.opts.outputKind = OutputKind::SharedLibrary; continue; }
        if (arg == "--emit-llvm") { job.opts.outputKind = OutputKind::LLVMIR; continue; }
        if (arg == "-S")  { job.opts.outputKind = OutputKind::Assembly; continue; }
        if (arg == "--check") { job.opts.stopAfter = PipelineStage::Sema; continue; }
        if (arg == "--dump-ast") { job.opts.dumpAST = true; continue; }
        if (arg == "--dump-ir")  { job.opts.dumpIR  = true; continue; }
        if (arg == "-Werror")    { job.cOpts.warningsAsErrors = true; continue; }
        if (arg == "--verbose")  { job.opts.verbose = true; continue; }
        if (arg == "-o") {
            if (++i >= argc) { fputs("error: -o needs a file\n", stderr); return false; }
            job.opts.outputFile = argv[i]; continue;
        }
        if (arg.size() > 2 && arg[0]=='-' && arg[1]=='o') {
            job.opts.outputFile = std::string(arg.substr(2)); continue;
        }
        if (arg == "--target") {
            if (++i >= argc) { fputs("error: --target needs a triple\n", stderr); return false; }
            job.opts.target = argv[i]; continue;
        }
        if (arg.size() > 2 && arg[0]=='-' && arg[1]=='I') {
            job.opts.includePaths.emplace_back(arg.substr(2)); continue;
        }
        if (arg.size() > 2 && arg[0]=='-' && arg[1]=='L') {
            job.opts.libraryPaths.emplace_back(arg.substr(2)); continue;
        }
        if (arg.size() > 2 && arg[0]=='-' && arg[1]=='l') {
            job.opts.libraries.emplace_back(arg.substr(2)); continue;
        }
        if (!arg.empty() && arg[0] != '-') {
            job.inputFiles.emplace_back(arg); continue;
        }
        fprintf(stderr, "vexc: unknown option: %.*s\n", (int)arg.size(), arg.data());
        return false;
    }
    return true;
}

// ── Compile one file ──────────────────────────────────────────────────────────

static int compileFile(const std::string& path, const Job& job, VexContext& vctx) {
    // ── Phase 1: Load source ──────────────────────────────────────────────────
    auto buf = MemoryBuffer::fromFile(path);
    if (!buf) {
        fprintf(stderr, "vexc: cannot open '%s'\n", path.c_str());
        return 1;
    }
    FileID fid = vctx.srcMgr().addBuffer(std::move(buf), path);
    const MemoryBuffer* mbuf = vctx.srcMgr().getBuffer(fid);
    if (!mbuf) return 1;

    // ── Phase 2: Lex ──────────────────────────────────────────────────────────
    Lexer lexer(*mbuf, fid, vctx.srcMgr(), vctx.diags());
    auto tokens = lexer.lexAll();
    if (vctx.diags().hasErrors()) return 1;

    // ── Phase 3: Parse ────────────────────────────────────────────────────────
    ASTContext astCtx;
    std::string moduleName = std::filesystem::path(path).stem().string();
    Parser parser(std::move(tokens), astCtx, vctx.srcMgr(), vctx.diags());
    ModuleDecl* mod = parser.parseModule(moduleName);
    if (vctx.diags().hasErrors() || !mod) return 1;

    // ── Dump AST ──────────────────────────────────────────────────────────────
    if (job.opts.dumpAST) {
        ASTDumper dumper(vctx.srcMgr(), stdout, /*color=*/true);
        dumper.dump(mod);
        return 0;
    }

    // ── Phase 4: Semantic Analysis ────────────────────────────────────────────
    Sema sema(vctx, astCtx);
    sema.run(mod);
    if (vctx.diags().hasErrors()) return 1;

    if (job.opts.stopAfter == PipelineStage::Sema) {
        fprintf(stdout, "vexc: %s — OK\n", path.c_str());
        return 0;
    }

    // ── Phase 5: IR Generation ────────────────────────────────────────────────
    IRGen irgen(vctx.diags());
    auto irMod = irgen.generate(mod);
    if (!irMod) {
        fprintf(stderr, "vexc: IR generation failed for '%s'\n", path.c_str());
        return 1;
    }

    if (job.opts.dumpIR) {
        fprintf(stdout, "; ── VEX IR: %s ─────────────────────────────\n", moduleName.c_str());
        irMod->dump(std::cerr);
    }
    if (job.opts.stopAfter == PipelineStage::IRGen) return 0;

    // ── Phase 6: LLVM Code Generation ────────────────────────────────────────
    CodeGen cg(vctx.diags(), job.opts);
    auto llvmMod = cg.lower(*irMod);
    if (!llvmMod) {
        fprintf(stderr, "vexc: code generation failed for '%s'\n", path.c_str());
        return 1;
    }

    if (job.opts.dumpIR) {
        fprintf(stdout, "\n; ── LLVM IR: %s ─────────────────────────────\n", moduleName.c_str());
        llvmMod->print(llvm::outs(), nullptr);
        llvm::outs().flush();
    }

    // ── Phase 7: Emit ─────────────────────────────────────────────────────────
    std::string outBase = job.opts.outputFile.empty() ? moduleName : job.opts.outputFile;

    switch (job.opts.outputKind) {
    case OutputKind::LLVMIR:
        return cg.emitLLVMIR(*llvmMod, outBase + ".ll") ? 0 : 1;
    case OutputKind::LLVMBC:
        return cg.emitBitcode(*llvmMod, outBase + ".bc") ? 0 : 1;
    case OutputKind::Assembly:
        return cg.emitAssembly(*llvmMod, outBase + ".s") ? 0 : 1;
    case OutputKind::ObjectFile:
        return cg.emitObjectFile(*llvmMod, outBase + ".o") ? 0 : 1;
    case OutputKind::VexIR:
        irMod->dump(std::cerr);
        return 0;
    default:
        // Full compile: object file then link
        std::string objPath = outBase + ".o";
        if (!cg.emitObjectFile(*llvmMod, objPath)) return 1;

        // ── Phase 8: Link ─────────────────────────────────────────────────────
        // Use ld.lld (or system linker) to produce an executable
        std::string rtPath;
        // Find runtime library beside the compiler binary
        char exePath[4096] = {};
        ssize_t len = readlink("/proc/self/exe", exePath, sizeof(exePath)-1);
        if (len > 0) {
            std::filesystem::path ep(exePath);
            auto rtCand = ep.parent_path().parent_path() / "lib" / "libvexruntime.so";
            if (std::filesystem::exists(rtCand)) rtPath = rtCand.string();
        }

        std::string linkerCmd = "ld.lld -o " + outBase +
            " " + objPath +
            " -dynamic-linker /lib64/ld-linux-x86-64.so.2" +
            " -lc";
        if (!rtPath.empty()) linkerCmd += " " + rtPath;
        if (job.opts.verbose)
            fprintf(stderr, "vexc: linking: %s\n", linkerCmd.c_str());

        int ret = system(linkerCmd.c_str());
        if (ret != 0) {
            // Try with system cc as fallback
            std::string ccCmd = "cc -o " + outBase + " " + objPath;
            if (!rtPath.empty()) ccCmd += " " + rtPath + " -Wl,-rpath," +
                                   std::filesystem::path(rtPath).parent_path().string();
            ret = system(ccCmd.c_str());
        }
        return (ret == 0) ? 0 : 1;
    }
}

// ── Entry point ───────────────────────────────────────────────────────────────

int main(int argc, char** argv) {
    Job job;
    if (!parseCLI(argc, argv, job)) return 0;

    if (job.inputFiles.empty()) {
        if (std::filesystem::exists("main.vex"))
            job.inputFiles.push_back("main.vex");
        else {
            fprintf(stderr, "vexc: no input files\n");
            printHelp(argv[0]);
            return 1;
        }
    }

    // Phase 1: collect options then compile.
    // The TextDiagnosticConsumer must reference the same SourceManager
    // that VexContext uses internally. We pass a shared one by pointer.
    // Build a temporary SourceManager for the consumer, then point it at
    // ctx.srcMgr() after ctx is constructed — this ensures error source
    // context rendering uses the same SourceManager the files are loaded into.
    static SourceManager tempSrcMgr;
    TextDiagnosticConsumer consumer(tempSrcMgr, /*useColor=*/true);
    VexContext ctx(consumer, job.cOpts);
    // Now re-point consumer at ctx's actual SourceManager
    consumer.setSrcMgr(ctx.srcMgr());

    int exitCode = 0;
    for (auto& file : job.inputFiles) {
        int r = compileFile(file, job, ctx);
        if (r != 0) exitCode = r;
        if (ctx.diags().shouldAbort()) break;
    }

    consumer.finish();
    return exitCode;
}
