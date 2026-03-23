#pragma once
// include/vex/Linker/VexLinker.h



// ============================================================================
// vex/Linker/VexLinker.h  — Invokes LLD to link object files
// ============================================================================
#include "vex/Linker/LinkerOptions.h"
#include "vex/Core/DiagnosticEngine.h"

namespace vex {

class VexLinker {
public:
    explicit VexLinker(DiagnosticEngine& diags) : diags_(diags) {}

    // Link using the options; returns true on success
    bool link(const LinkerOptions& opts);

    // Determine correct LLD flavor from triple
    static std::string flavorForTriple(std::string_view triple);

private:
    DiagnosticEngine& diags_;

    bool linkELF(const LinkerOptions& opts);
    bool linkCOFF(const LinkerOptions& opts);
    bool linkMachO(const LinkerOptions& opts);
    bool linkWasm(const LinkerOptions& opts);

    std::vector<const char*> buildArgs(const LinkerOptions& opts);
};
} // namespace vex
