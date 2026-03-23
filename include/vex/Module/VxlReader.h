#pragma once
// include/vex/Module/VxlReader.h



// vex/Module/VxlReader.h  — Deserializes a .vxl file back to AST
#include "vex/Module/VxlFormat.h"
#include "vex/Core/MemoryBuffer.h"
#include "vex/AST/ASTContext.h"
#include "vex/Core/Error.h"
#include <string_view>

namespace vex {

class VxlReader {
public:
    VxlReader() = default;

    // Read a module from a .vxl file; returns nullptr on failure
    ModuleDecl* read(MemoryBuffer& buf, ASTContext& ctx);

    static Result<void> validate(MemoryBuffer& buf);

private:
    ModuleDecl* parseHeader(const uint8_t* data, size_t size, ASTContext& ctx);
    ModuleDecl* parseAST(const uint8_t* data, size_t size, ASTContext& ctx);
    bool parseExports(const uint8_t* data, size_t size, ModuleDecl* mod);
    bool parseImports(const uint8_t* data, size_t size, ModuleDecl* mod);
};
} // namespace vex
