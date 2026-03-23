#pragma once
// include/vex/Module/VxlWriter.h



// vex/Module/VxlWriter.h  — Serializes a module to a .vxl file
#include "vex/Module/VxlFormat.h"
#include "vex/AST/Decl.h"
#include "vex/Core/Error.h"
#include <string_view>
#include <vector>

namespace vex {

class VxlWriter {
public:
    VxlWriter() = default;

    // Serialize a module to bytes
    std::vector<uint8_t> write(ModuleDecl* mod);

    // Write to a file
    Result<void> writeTo(ModuleDecl* mod, std::string_view path);

private:
    void writeHeader(ModuleDecl* mod, std::vector<uint8_t>& out);
    void writeAST(ModuleDecl* mod, std::vector<uint8_t>& out);
    void writeExports(ModuleDecl* mod, std::vector<uint8_t>& out);
    void writeImports(ModuleDecl* mod, std::vector<uint8_t>& out);

    void writeUInt32(std::vector<uint8_t>& out, uint32_t v);
    void writeString(std::vector<uint8_t>& out, std::string_view s);
};
} // namespace vex
