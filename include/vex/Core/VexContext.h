#pragma once
// ============================================================================
// vex/Core/VexContext.h
// The global compiler context.
// Owns SourceManager, DiagnosticEngine, and shared resources.
// Every compiler phase receives a VexContext reference.
// ============================================================================

#include "vex/Core/DiagnosticEngine.h"
#include "vex/Core/DiagnosticConsumer.h"
#include "vex/Core/SourceManager.h"
#include "vex/Core/Platform.h"

#include <memory>
#include <string>

namespace vex {

// ── CompilerOptions ───────────────────────────────────────────────────────────
// All command-line flags and build settings in one place.

struct CompilerOptions {
    // Target
    TargetOS   targetOS   = TargetOS::Linux;
    TargetArch targetArch = TargetArch::X86_64;

    // Optimization level (matches VEX build flags)
    enum class OptLevel : uint8_t { O0, O1, O2, O3, Os };
    OptLevel optLevel = OptLevel::O0;

    // Debug info
    bool emitDebugInfo = false;

    // Output kind
    enum class OutputKind : uint8_t {
        Executable,     // native binary
        VxlLibrary,     // .vxl compiled library
        SharedLib,      // .so / .dll
        WasmModule,     // .wasm
        LLVMBitcode,    // .bc (for debugging)
        LLVMAssembly,   // .ll  (for debugging)
        ObjectFile,     // .o
    };
    OutputKind outputKind = OutputKind::Executable;

    // Paths
    std::string outputPath;
    std::string projectRoot;
    std::string stdlibPath;

    // Feature flags (match VEX -D flags)
    bool disableAssert    = false;  // -D DISABLE_ASSERT=true
    bool enablePolly      = false;  // --polly
    bool noRuntime        = false;  // --no-rt
    bool warningsAsErrors = false;  // -Werror

    // Build mode derived flags
    bool isDebugBuild()   const { return optLevel == OptLevel::O0; }
    bool isReleaseBuild() const {
        return optLevel == OptLevel::O2 || optLevel == OptLevel::O3;
    }

    // Target string for comptime #TARGET variable
    std::string targetString() const {
        std::string s(targetOSName(targetOS));
        s += '-';
        s += targetArchName(targetArch);
        return s;
    }
};

// ── VexContext ────────────────────────────────────────────────────────────────

class VexContext {
public:
    // Create with a consumer for diagnostics (usually TextDiagnosticConsumer)
    explicit VexContext(DiagnosticConsumer& consumer,
                        CompilerOptions     opts = {});

    ~VexContext() = default;

    // Non-copyable
    VexContext(const VexContext&) = delete;
    VexContext& operator=(const VexContext&) = delete;

    // ── Accessors ─────────────────────────────────────────────────────────────

    SourceManager&     srcMgr()   { return srcMgr_; }
    const SourceManager& srcMgr() const { return srcMgr_; }

    DiagnosticEngine&  diags()    { return diagEngine_; }
    const DiagnosticEngine& diags() const { return diagEngine_; }

    const CompilerOptions& opts() const { return opts_; }
    CompilerOptions&       opts()       { return opts_; }

    // ── Convenience diagnostic shortcuts ─────────────────────────────────────

    DiagnosticBuilder report(DiagID id, SourceLocation loc, std::string msg) {
        return diagEngine_.report(id, loc, std::move(msg));
    }

    bool hasErrors() const { return diagEngine_.hasErrors(); }
    bool shouldAbort() const { return diagEngine_.shouldAbort(); }

private:
    CompilerOptions  opts_;
    SourceManager    srcMgr_;
    DiagnosticEngine diagEngine_;
};

} // namespace vex