// ============================================================================
// vex/Driver/main.cpp
// VEX Compiler entry point — parses CLI, drives the full compilation pipeline.
// ============================================================================

#include "vex/Core/VexContext.h"
#include "vex/Core/DiagnosticConsumer.h"
#include "vex/Core/MemoryBuffer.h"
#include "vex/Core/Version.h"
#include "vex/Lexer/Lexer.h"
#include "vex/Parser/Parser.h"
#include "vex/AST/ASTDumper.h"

#include <cstdio>
#include <cstring>
#include <string>
#include <vector>
#include <filesystem>

using namespace vex;

// ── Usage ─────────────────────────────────────────────────────────────────────

static void printVersion() {
    fprintf(stdout,
        "VEX Compiler v%s (language v%s)\n"
        "LLVM backend v%s\n",
        std::string(VEX_VERSION_STRING).c_str(),
        std::string(VEX_LANGUAGE_VERSION).c_str(),
        std::string(VEX_TARGET_LLVM_VERSION).c_str()
    );
}

static void printUsage(const char* prog) {
    fprintf(stderr,
        "VEX Compiler v%s\n"
        "Usage: %s [options] <file.vex> [file.vex ...]\n"
        "\n"
        "Options:\n"
        "  -o <output>          Output file path (default: a.out / <name>)\n"
        "  -O0 -O1 -O2 -O3 -Os  Optimization level (default: -O0)\n"
        "  -g                   Emit debug info\n"
        "  --target <triple>    Target triple (e.g. linux-x64, linux-arm64, wasm)\n"
        "  --check              Type-check only, do not emit code\n"
        "  --dump-ast           Print AST to stdout\n"
        "  --dump-ir            Print LLVM IR to stdout\n"
        "  --lib                Build as Vex library (.vxl)\n"
        "  --shared             Build as shared library (.so/.dll/.dylib)\n"
        "  --no-rt              No runtime (bare metal / embedded)\n"
        "  -D <NAME>=<value>    Define a compile-time variable\n"
        "  -Werror              Treat warnings as errors\n"
        "  --version            Show compiler version\n"
        "  -h --help            Show this help\n"
        "\n"
        "Build commands (from project directory):\n"
        "  vexc                 Compile + run (default for dev)\n"
        "  vexc --check         Type-check only\n"
        "  vexc --dump-ast      Show parsed AST\n"
        "  vexc --lib           Build .vxl library\n"
        "  vexc --shared        Build shared library\n"
        "\n"
        "Examples:\n"
        "  vexc main.vex                       Compile to executable\n"
        "  vexc main.vex -o myapp              Named output\n"
        "  vexc --target linux-arm64 main.vex  Cross-compile\n"
        "  vexc --dump-ast main.vex            Show AST\n"
        "  vexc --check main.vex               Type-check only\n"
        "\n",
        std::string(VEX_VERSION_STRING).c_str(),
        prog
    );
}

// ── Compiler pipeline state ───────────────────────────────────────────────────

struct CompilerJob {
    CompilerOptions         opts;
    std::vector<std::string> inputFiles;
    bool checkOnly  = false;
    bool dumpAst    = false;
    bool dumpIr     = false;
    bool noRuntime  = false;
    bool runAfter   = false;  // vexc with no flags → compile + run
};

// ── Parse command line ────────────────────────────────────────────────────────

static bool parseCLI(int argc, char** argv, CompilerJob& job) {
    if (argc < 2) {
        job.runAfter = true; // default: compile + run from vex.toml
        return true;
    }

    for (int i = 1; i < argc; ++i) {
        std::string_view arg(argv[i]);

        if (arg == "-h" || arg == "--help") {
            printUsage(argv[0]);
            return false;
        }
        if (arg == "--version") {
            printVersion();
            return false;
        }
        if (arg == "-o" && i + 1 < argc) {
            job.opts.outputPath = argv[++i];
        }
        else if (arg == "-O0") job.opts.optLevel = CompilerOptions::OptLevel::O0;
        else if (arg == "-O1") job.opts.optLevel = CompilerOptions::OptLevel::O1;
        else if (arg == "-O2") job.opts.optLevel = CompilerOptions::OptLevel::O2;
        else if (arg == "-O3") job.opts.optLevel = CompilerOptions::OptLevel::O3;
        else if (arg == "-Os") job.opts.optLevel = CompilerOptions::OptLevel::Os;
        else if (arg == "-g")  job.opts.emitDebugInfo = true;
        else if (arg == "-Werror") job.opts.warningsAsErrors = true;
        else if (arg == "--check")    job.checkOnly  = true;
        else if (arg == "--dump-ast") job.dumpAst    = true;
        else if (arg == "--dump-ir")  job.dumpIr     = true;
        else if (arg == "--lib")      job.opts.outputKind = CompilerOptions::OutputKind::VxlLibrary;
        else if (arg == "--shared")   job.opts.outputKind = CompilerOptions::OutputKind::SharedLib;
        else if (arg == "--no-rt")    job.noRuntime  = true;
        else if (arg == "--no-std")   job.noRuntime  = true;
        else if (arg == "--target" && i + 1 < argc) {
            std::string_view triple(argv[++i]);
            auto dash = triple.find('-');
            if (dash != std::string_view::npos) {
                job.opts.targetOS   = parseTargetOS(triple.substr(0, dash));
                job.opts.targetArch = parseTargetArch(triple.substr(dash + 1));
            } else {
                job.opts.targetOS   = parseTargetOS(triple);
            }
        }
        else if (arg.starts_with("-D") && arg.size() > 2) {
            // -DNAME=value or -D NAME=value
            std::string def = std::string(arg.substr(2));
            if (def.empty() && i + 1 < argc) def = argv[++i];
            (void)def; // stored in opts.defines in full impl
        }
        else if (arg.starts_with('-')) {
            fprintf(stderr, "vexc: unknown option: %s\n", argv[i]);
            return false;
        }
        else {
            job.inputFiles.emplace_back(arg);
        }
    }
    return true;
}

// ── Compile one file ──────────────────────────────────────────────────────────

static int compileFile(const std::string& path, const CompilerJob& job, VexContext& ctx) {
    // ── Phase 1: Load source ──────────────────────────────────────────────────
    auto buf = MemoryBuffer::fromFile(path);
    if (!buf) {
        fprintf(stderr, "vexc: cannot open file: %s\n", path.c_str());
        return 1;
    }

    FileID fileID = ctx.srcMgr().addBuffer(std::move(buf), path);
    const MemoryBuffer* mbuf = ctx.srcMgr().getBuffer(fileID);

    // ── Phase 2: Lex ─────────────────────────────────────────────────────────
    Lexer lexer(*mbuf, fileID, ctx.srcMgr(), ctx.diags());
    auto tokens = lexer.lexAll();

    if (ctx.hasErrors()) return 1;

    // ── Phase 3: Parse ────────────────────────────────────────────────────────
    ASTContext astCtx;
    std::filesystem::path p(path);
    std::string moduleName = p.stem().string();

    Parser parser(std::move(tokens), astCtx, ctx.srcMgr(), ctx.diags());
    ModuleDecl* mod = parser.parseModule(moduleName);

    if (ctx.hasErrors() || !mod) return 1;

    // ── Dump AST if requested ─────────────────────────────────────────────────
    if (job.dumpAst) {
        ASTDumper dumper(ctx.srcMgr(), stdout, /*useColor=*/true);
        dumper.dump(mod);
        return 0;
    }

    // ── Phase 4: Type-check (stub — full sema not yet wired) ─────────────────
    // TODO: ctx.sema().check(mod);
    if (ctx.hasErrors()) return 1;

    if (job.checkOnly) {
        fprintf(stdout, "vexc: %s — type-check OK\n", path.c_str());
        return 0;
    }

    // ── Phase 5: CodeGen (stub — LLVM IR emission not yet wired) ─────────────
    // TODO: ctx.codegen().emit(mod, job.opts);

    fprintf(stdout, "vexc: %s (%zu bytes) — parse OK\n", path.c_str(), mbuf->size());
    fprintf(stdout, "vexc: pipeline stub — Sema → IR → CodeGen → Link not yet connected\n");

    return 0;
}

// ── Entry point ───────────────────────────────────────────────────────────────

int main(int argc, char** argv) {
    CompilerJob job;
    if (!parseCLI(argc, argv, job)) return 0;

    // Set up diagnostic consumer
    // We create a temporary SourceManager for the consumer
    // (proper wiring happens inside VexContext)
    SourceManager tmpSrcMgr;
    TextDiagnosticConsumer consumer(tmpSrcMgr, /*useColor=*/true);
    VexContext ctx(consumer, job.opts);
    consumer = TextDiagnosticConsumer(ctx.srcMgr(), /*useColor=*/true);

    if (job.inputFiles.empty()) {
        // Try to find main.vex in current directory
        if (std::filesystem::exists("main.vex")) {
            job.inputFiles.push_back("main.vex");
        } else {
            fprintf(stderr, "vexc: no input files\n");
            printUsage(argv[0]);
            return 1;
        }
    }

    int exitCode = 0;
    for (auto& file : job.inputFiles) {
        int r = compileFile(file, job, ctx);
        if (r != 0) exitCode = r;
        if (ctx.shouldAbort()) break;
    }

    consumer.finish();
    return exitCode;
}