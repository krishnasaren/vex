// src/Driver/Options.cpp — Parse argv into DriverOptions
#include "vex/Driver/Options.h"
#include <cstring>
#include <cstdio>

namespace vex {

static void printHelp() {
    fputs(
        "VEX Compiler v1.0.0  (Language 4.0 · LLVM 18)\n\n"
        "Usage:\n"
        "  vexc <inputs>           Compile and link\n"
        "  vexc -c <inputs>        Compile to object files only\n"
        "  vexc --check <input>    Semantic check only\n"
        "  vexc --version          Show version\n"
        "  vexc --help             Show this help\n\n"
        "Output options:\n"
        "  -o <file>               Output file\n"
        "  --emit-llvm             Emit LLVM IR (.ll)\n"
        "  --emit-bc               Emit LLVM bitcode (.bc)\n"
        "  -S                      Emit assembly (.s)\n"
        "  -c                      Emit object file (.o), no link\n"
        "  --emit-vexir            Emit VEX IR (text)\n"
        "  -shared                 Build shared library\n"
        "  -static                 Link statically\n\n"
        "Optimisation:\n"
        "  -O0  -O1  -O2  -O3     Optimisation level (default -O0)\n"
        "  -Os                     Optimise for size\n"
        "  --lto                   Link-time optimisation\n\n"
        "Debugging:\n"
        "  -g                      Emit debug information\n"
        "  --dump-ast              Print AST\n"
        "  --dump-ir               Print VEX IR and LLVM IR\n"
        "  --verify-ir             Verify LLVM IR\n\n"
        "Target:\n"
        "  --target <triple>       Set target triple\n\n"
        "Warnings:\n"
        "  -Werror                 Treat warnings as errors\n"
        "  -w                      Suppress all warnings\n\n"
        "Paths:\n"
        "  -I <dir>                Add include directory\n"
        "  -L <dir>                Add library search path\n"
        "  -l <name>               Link library\n\n"
        "Sanitizers:\n"
        "  --sanitize=address      AddressSanitizer\n"
        "  --sanitize=undefined    UBSan\n"
        "  --sanitize=thread       ThreadSanitizer\n",
        stdout);
}

static void printVersion() {
    fputs("vexc 1.0.0 (VEX language 4.0, LLVM 18)\n", stdout);
}

DriverOptions parseOptions(int argc, const char* const* argv,
                            bool& shouldExit, int& exitCode) {
    DriverOptions opts;
    shouldExit = false;
    exitCode   = 0;

    auto arg_sv = [&](int i) -> std::string_view {
        return (i < argc) ? std::string_view(argv[i]) : std::string_view{};
    };

    for (int i = 1; i < argc; ++i) {
        std::string_view arg = argv[i];

        if (arg == "--help" || arg == "-h") {
            printHelp(); shouldExit = true; return opts;
        }
        if (arg == "--version") {
            printVersion(); shouldExit = true; return opts;
        }

        // Output
        if (arg == "-o") {
            if (++i >= argc) { fputs("error: -o requires a file\n", stderr); exitCode=1; shouldExit=true; return opts; }
            opts.outputFile = argv[i]; continue;
        }
        if (arg.substr(0, 2) == "-o") { opts.outputFile = arg.substr(2); continue; }

        // Optimisation
        if (arg == "-O0") { opts.optLevel = 0; continue; }
        if (arg == "-O1") { opts.optLevel = 1; continue; }
        if (arg == "-O2") { opts.optLevel = 2; continue; }
        if (arg == "-O3") { opts.optLevel = 3; continue; }
        if (arg == "-Os" || arg == "-Oz") { opts.optLevel = 2; continue; }

        // Debug info
        if (arg == "-g") { opts.emitDebugInfo = true; continue; }

        // Target
        if (arg == "--target") {
            if (++i >= argc) { fputs("error: --target requires a triple\n", stderr); exitCode=1; shouldExit=true; return opts; }
            opts.target = argv[i]; continue;
        }

        // Emit mode → OutputKind
        if (arg == "--emit-llvm")  { opts.outputKind = OutputKind::LLVMIR;    continue; }
        if (arg == "--emit-bc")    { opts.outputKind = OutputKind::LLVMBC;    continue; }
        if (arg == "-S")           { opts.outputKind = OutputKind::Assembly;  continue; }
        if (arg == "--emit-vexir") { opts.outputKind = OutputKind::VexIR;     continue; }

        // Compile only
        if (arg == "-c") {
            opts.outputKind = OutputKind::ObjectFile;
            continue;
        }

        // Check only
        if (arg == "--check") { opts.stopAfter = PipelineStage::Sema; continue; }

        // Dump
        if (arg == "--dump-ast") { opts.dumpAST = true; continue; }
        if (arg == "--dump-ir")  { opts.dumpIR  = true; continue; }
        if (arg == "--verify-ir"){ opts.verifyIR = true; continue; }

        // Linking
        if (arg == "-shared")    { opts.outputKind = OutputKind::SharedLibrary; continue; }
        if (arg == "--lto")      { opts.enableLTO  = true; continue; }
        if (arg == "--polly")    { opts.enablePolly = true; continue; }

        // Paths
        if (arg.substr(0, 2) == "-I") { opts.includePaths.emplace_back(arg.substr(2)); continue; }
        if (arg.substr(0, 2) == "-L") { opts.libraryPaths.emplace_back(arg.substr(2)); continue; }
        if (arg.substr(0, 2) == "-l") { opts.libraries.emplace_back(arg.substr(2));    continue; }

        // Misc
        if (arg == "--verbose")            { opts.verbose = true; continue; }
        if (arg == "-Werror")              { continue; } // warningsAsErrors not in DriverOptions
        if (arg == "-w")                   { opts.quietDiags = true; continue; }
        if (arg == "--no-color")           { opts.colorDiags = false; continue; }
        if (arg == "--show-timings")       { opts.showTimings = true; continue; }
        if (arg.substr(0, 11) == "--sanitize=") {
            auto s = arg.substr(11);
            if (s == "address")   opts.asan = true;
            if (s == "undefined") opts.ubsan = true;
            if (s == "thread")    opts.tsan = true;
            if (s == "memory")    opts.msan = true;
            continue;
        }

        // Input files
        if (!arg.empty() && arg[0] != '-') {
            opts.inputFiles.emplace_back(arg); continue;
        }

        fprintf(stderr, "error: unknown option '%.*s'\n", (int)arg.size(), arg.data());
        exitCode = 1; shouldExit = true; return opts;
    }

    return opts;
}

} // namespace vex
