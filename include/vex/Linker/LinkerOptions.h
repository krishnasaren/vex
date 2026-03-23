#pragma once
// include/vex/Linker/LinkerOptions.h



// vex/Linker/LinkerOptions.h
#include <string>
#include <vector>
namespace vex {
struct LinkerOptions {
    std::string              outputPath;
    std::string              flavor;       // "lld-elf", "lld-coff", "lld-wasm"
    std::string              target;       // LLVM triple
    std::vector<std::string> inputObjects;
    std::vector<std::string> libraryPaths;
    std::vector<std::string> libraries;
    std::vector<std::string> extraFlags;
    bool                     shared    = false;
    bool                     static_   = false;
    bool                     pie       = false;
    bool                     lto       = false;
    bool                     verbose   = false;
    std::string              soname;       // -soname for shared libs
    std::string              rpath;
    std::string              entryPoint;   // override default _start
};
} // namespace vex











