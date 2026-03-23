#pragma once
// include/vex/Module/TomlParser.h



// ============================================================================
// vex/Module/TomlParser.h  — Minimal TOML parser for vex.toml
// ============================================================================
#include "vex/Module/ModuleManifest.h"
#include "vex/Core/Error.h"
#include <string_view>

namespace vex {

class TomlParser {
public:
    // Parse vex.toml contents into a ModuleManifest
    static Result<ModuleManifest> parse(std::string_view source,
                                         std::string_view filename);
private:
    struct Parser {
        std::string_view src;
        size_t           pos  = 0;
        std::string      file;

        std::string parseString();
        bool        parseBool();
        std::string parseVersion();
        void        skipWhitespaceAndComments();
        bool        expect(char c);
        std::string parseKey();
        std::string parseValue();
    };
};
} // namespace vex
