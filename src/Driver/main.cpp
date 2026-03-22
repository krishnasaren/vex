// ============================================================================
// vex/Driver/main.cpp
// VEX compiler entry point.
// Parses command-line args, sets up context, drives compilation.
// ============================================================================

#include "vex/Core/VexContext.h"
#include "vex/Core/DiagnosticConsumer.h"
#include "vex/Core/MemoryBuffer.h"

#include <cstdio>
#include <cstring>
#include <string>
#include <vector>

using namespace vex;

// ── Minimal driver ────────────────────────────────────────────────────────────

static void printUsage(const char* prog) {
    fprintf(stderr,
        "VEX Compiler v1.0.0\n"
        "Usage: %s [options] <file.vex>\n"
        "\n"
        "Options:\n"
        "  -o <output>     Output file path\n"
        "  -O0 -O1 -O2 -O3 -Os   Optimization level (default: -O0)\n"
        "  -g              Emit debug info\n"
        "  --target <triple>  Target triple (e.g. linux-x64, wasm)\n"
        "  --check         Type-check only, do not emit code\n"
        "  --dump-ast      Print AST to stdout\n"
        "  --dump-ir       Print LLVM IR to stdout\n"
        "  -Werror         Treat warnings as errors\n"
        "  -h --help       Show this help\n"
        "\n",
        prog
    );
}

int main(int argc, char** argv) {
    if (argc < 2) {
        printUsage(argv[0]);
        return 1;
    }

    // ── Parse command line ────────────────────────────────────────────────────
    CompilerOptions opts;
    std::vector<std::string> inputFiles;
    bool checkOnly = false;
    bool dumpAst   = false;
    bool dumpIr    = false;

    for (int i = 1; i < argc; ++i) {
        std::string_view arg(argv[i]);

        if (arg == "-h" || arg == "--help") {
            printUsage(argv[0]);
            return 0;
        } else if (arg == "-o" && i + 1 < argc) {
            opts.outputPath = argv[++i];
        } else if (arg == "-O0") {
            opts.optLevel = CompilerOptions::OptLevel::O0;
        } else if (arg == "-O1") {
            opts.optLevel = CompilerOptions::OptLevel::O1;
        } else if (arg == "-O2") {
            opts.optLevel = CompilerOptions::OptLevel::O2;
        } else if (arg == "-O3") {
            opts.optLevel = CompilerOptions::OptLevel::O3;
        } else if (arg == "-Os") {
            opts.optLevel = CompilerOptions::OptLevel::Os;
        } else if (arg == "-g") {
            opts.emitDebugInfo = true;
        } else if (arg == "-Werror") {
            opts.warningsAsErrors = true;
        } else if (arg == "--check") {
            checkOnly = true;
        } else if (arg == "--dump-ast") {
            dumpAst = true;
        } else if (arg == "--dump-ir") {
            dumpIr = true;
        } else if (arg == "--target" && i + 1 < argc) {
            std::string_view triple(argv[++i]);
            // Parse "linux-x64" or "wasm" etc.
            auto dash = triple.find('-');
            if (dash != std::string_view::npos) {
                opts.targetOS   = parseTargetOS(triple.substr(0, dash));
                opts.targetArch = parseTargetArch(triple.substr(dash + 1));
            } else {
                opts.targetOS   = parseTargetOS(triple);
            }
        } else if (arg.starts_with('-')) {
            fprintf(stderr, "vexc: unknown option: %s\n", argv[i]);
            return 1;
        } else {
            inputFiles.emplace_back(arg);
        }
    }

    if (inputFiles.empty()) {
        fprintf(stderr, "vexc: no input files\n");
        printUsage(argv[0]);
        return 1;
    }

    // ── Set up compiler context ────────────────────────────────────────────────
    TextDiagnosticConsumer consumer(
        // We need a SourceManager to print source context in diagnostics.
        // Create a temporary one — the real one lives in VexContext.
        // For now, we pass a stub.
        *reinterpret_cast<SourceManager*>(nullptr),  // placeholder — TODO
        /*useColor=*/true
    );

    // TODO: wire up properly when full pipeline is connected
    // VexContext ctx(consumer, std::move(opts));

    // ── For now: just verify we can load the input file ───────────────────────
    for (auto& path : inputFiles) {
        auto buf = MemoryBuffer::fromFile(path);
        if (!buf) {
            fprintf(stderr, "vexc: cannot open file: %s\n", path.c_str());
            return 1;
        }
        fprintf(stdout, "vexc: loaded %s (%zu bytes)\n",
                path.c_str(), buf->size());
    }

    fprintf(stdout,
        "vexc: pipeline not yet connected — "
        "this will invoke Lexer → Parser → Sema → CodeGen → Linker\n"
    );

    return 0;
}