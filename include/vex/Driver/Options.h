#pragma once
// include/vex/Driver/Options.h


// ============================================================================
// vex/Driver/Options.h  — All parsed command-line options for the driver
// ============================================================================
#include "vex/Driver/BuildMode.h"
#include "vex/Driver/OutputKind.h"
#include "vex/Driver/PipelineStage.h"
#include "vex/Driver/TargetInfo.h"
#include "vex/Driver/TargetOptions.h"
#include <string>
#include <vector>
#include <optional>

namespace vex {

struct DriverOptions {
    // Input / output
    std::vector<std::string> inputFiles;
    std::string              outputFile;
    std::vector<std::string> includePaths;
    std::vector<std::string> libraryPaths;
    std::vector<std::string> libraries;

    // Compilation controls
    BuildMode                buildMode   = BuildMode::Debug;
    OutputKind               outputKind  = OutputKind::Executable;
    std::optional<PipelineStage> stopAfter;
    std::string              target;       // LLVM triple or "host"
    TargetOptions            targetOpts;

    // Verbosity / diagnostics
    bool verbose          = false;
    bool quietDiags       = false;
    bool colorDiags       = true;
    bool showTimings      = false;

    // Features
    bool enablePolly      = false;
    bool enableLTO        = false;
    bool enablePGO        = false;
    std::string pgoProfile;

    // Debugging
    bool emitDebugInfo    = false;
    bool verifyIR         = false;
    bool dumpAST          = false;
    bool dumpIR           = false;
    bool dumpLLVMIR       = false;

    // Sanitizers
    bool asan  = false;
    bool ubsan = false;
    bool tsan  = false;
    bool msan  = false;

    // Optimization level 0-3 (derived from buildMode but overridable)
    int optLevel          = 0;

    // Linker
    std::string linker;   // "lld", "ld", "gold", ...
    std::vector<std::string> linkerFlags;

    // Parse from argc/argv; returns false on error
    bool parse(int argc, char** argv);
    void printHelp();
    void printVersion();
};

} // namespace vex
