cmake_minimum_required(VERSION 3.20)
project(vexcompiler VERSION 1.0.0 LANGUAGES C CXX)

# ── C++ Standard ─────────────────────────────────────────────────────────────
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# ── LLVM components used by the VEX compiler ─────────────────────────────────

set(VEX_LLVM_COMPONENTS
    Core
    Support
    Target
    TargetParser
    X86CodeGen
    X86AsmParser
    X86Desc
    X86Info
    AArch64CodeGen
    AArch64AsmParser
    AArch64Desc
    AArch64Info
    WebAssemblyCodeGen
    WebAssemblyAsmParser
    WebAssemblyDesc
    WebAssemblyInfo
    Passes
    Analysis
    BitWriter
    BitReader
    IRReader
    Linker
    LTO
    MC
    MCParser
    MCDisassembler
    Object
    Option
    ProfileData
    Remarks
    TextAPI
    TransformUtils
    ScalarOpts
    InstCombine
    AggressiveInstCombine
    IPO
    Vectorize
    Coroutines
    ObjCARCOpts
    CodeGen
    SelectionDAG
    AsmPrinter
    DebugInfoDWARF
    DebugInfoPDB
    DebugInfoMSF
    DebugInfoCodeView
    SymbolTablePrinter
)

# Map component names to actual LLVM library targets
llvm_map_components_to_libnames(VEX_LLVM_LIBS ${VEX_LLVM_COMPONENTS})

# ── Source files ──────────────────────────────────────────────────────────────

set(VEX_CORE_SOURCES
    src/Core/Assert.cpp
    src/Core/Platform.cpp
    src/Core/MemoryBuffer.cpp
    src/Core/SourceManager.cpp
    src/Core/DiagnosticIDs.cpp
    src/Core/DiagnosticEngine.cpp
    src/Core/DiagnosticConsumer.cpp
    src/Core/VexContext.cpp
)

set(VEX_LEXER_SOURCES
    src/Lexer/TokenKind.cpp
    src/Lexer/Lexer.cpp
)

set(VEX_PARSER_SOURCES
    src/Parser/Precedence.cpp
    # src/Parser/Parser.cpp        -- to be added
    # src/Parser/ParseDecl.cpp
    # src/Parser/ParseStmt.cpp
    # src/Parser/ParseExpr.cpp
    # src/Parser/ParseType.cpp
    # src/Parser/ParsePattern.cpp
    # src/Parser/ParseImport.cpp
)

set(VEX_AST_SOURCES
    src/AST/Visibility.cpp
    # src/AST/ASTDumper.cpp        -- to be added
    # src/AST/ASTSerializer.cpp
)

set(VEX_DRIVER_SOURCES
    src/Driver/main.cpp
)

# All compiler sources (everything except main.cpp — for library)
set(VEX_LIB_SOURCES
    ${VEX_CORE_SOURCES}
    ${VEX_LEXER_SOURCES}
    ${VEX_PARSER_SOURCES}
    ${VEX_AST_SOURCES}
)

# ── Compiler library (vexlib) ─────────────────────────────────────────────────
# Separate from the executable so unit tests can link against it.

add_library(vexlib STATIC ${VEX_LIB_SOURCES})

target_include_directories(vexlib
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/include
        ${LLVM_INCLUDE_DIRS}
)

target_compile_definitions(vexlib
    PUBLIC
        ${LLVM_DEFINITIONS}
)

target_link_libraries(vexlib
    PUBLIC
        ${VEX_LLVM_LIBS}
)

target_compile_features(vexlib PUBLIC cxx_std_17)

# Compiler warnings
if (CMAKE_CXX_COMPILER_ID MATCHES "Clang|GNU")
    target_compile_options(vexlib PRIVATE
        -Wall -Wextra -Wshadow -Wno-unused-parameter
    )
elseif (MSVC)
    target_compile_options(vexlib PRIVATE /W4 /wd4100)
endif()

# ── Compiler executable (vexc) ────────────────────────────────────────────────

add_llvm_tool(vexc ${VEX_DRIVER_SOURCES})

target_link_libraries(vexc
    PRIVATE
        vexlib
        ${VEX_LLVM_LIBS}
        # lld linker integration (enabled after Linker/ phase is written)
        # lldELF
        # lldCOFF
        # lldMachO
        # lldWasm
        # lldCommon
)

target_include_directories(vexc
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/include
        ${LLVM_INCLUDE_DIRS}
)

target_compile_features(vexc PRIVATE cxx_std_17)

# ── Polly integration (optional — enabled via -DVEX_ENABLE_POLLY=ON) ─────────

option(VEX_ENABLE_POLLY "Enable Polly loop optimizer integration" OFF)

if (VEX_ENABLE_POLLY)
    find_package(Polly QUIET)
    if (Polly_FOUND OR TARGET Polly)
        target_link_libraries(vexlib PUBLIC Polly LLVMPolly)
        target_compile_definitions(vexlib PUBLIC VEX_HAS_POLLY=1)
        message(STATUS "VEX: Polly integration enabled")
    else()
        message(WARNING "VEX_ENABLE_POLLY=ON but Polly was not found")
    endif()
endif()

# ── Install ───────────────────────────────────────────────────────────────────

install(TARGETS vexc
    RUNTIME DESTINATION bin
)

install(TARGETS vexlib
    ARCHIVE DESTINATION lib
)

install(DIRECTORY include/vex
    DESTINATION include
    FILES_MATCHING PATTERN "*.h"
)

# ── Status messages ───────────────────────────────────────────────────────────

message(STATUS "VEX Compiler v${PROJECT_VERSION}")
message(STATUS "VEX: LLVM ${LLVM_VERSION} found at ${LLVM_DIR}")
message(STATUS "VEX: Target: ${CMAKE_SYSTEM_NAME} ${CMAKE_SYSTEM_PROCESSOR}")
message(STATUS "VEX: Build type: ${CMAKE_BUILD_TYPE}")
message(STATUS "VEX: Polly: ${VEX_ENABLE_POLLY}")