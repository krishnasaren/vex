#pragma once
// include/vex/Linker/SymbolResolver.h

// ============================================================================
// vex/Linker/SymbolResolver.h  — Symbol resolution before linking
// ============================================================================
#include "vex/Core/DiagnosticEngine.h"
#include <string>
#include <string_view>
#include <unordered_map>
#include <vector>

namespace vex {

struct LinkerSymbol {
    std::string name;
    std::string sourceFile;
    bool        isDefined = false;
    bool        isWeak    = false;
    bool        isExport  = false;
};

class SymbolResolver {
public:
    explicit SymbolResolver(DiagnosticEngine& diags) : diags_(diags) {}

    void define(std::string_view name, std::string_view source,
                bool isWeak = false);
    void use(std::string_view name, std::string_view source);

    // Returns list of unresolved symbols
    std::vector<std::string> unresolvedSymbols() const;

    // Check for multiply-defined non-weak symbols
    bool checkDuplicates();

    bool hasErrors() const;

private:
    DiagnosticEngine& diags_;
    std::unordered_map<std::string, LinkerSymbol> defined_;
    std::unordered_map<std::string, std::vector<std::string>> used_;
};
} // namespace vex
