cmake_minimum_required(VERSION 3.20)
project(vexcompiler VERSION 1.0.0 LANGUAGES C CXX)

# ── C++ Standard ─────────────────────────────────────────────────────────────
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# ── CMake module path ─────────────────────────────────────────────────────────
list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")

# ── Options ───────────────────────────────────────────────────────────────────
include(VexOptions)

# ── Find LLVM ─────────────────────────────────────────────────────────────────
find_package(LLVM REQUIRED CONFIG)

message(STATUS "VEX Compiler v${PROJECT_VERSION}")
message(STATUS "VEX: LLVM ${LLVM_VERSION} found at ${LLVM_DIR}")
message(STATUS "VEX: Target: ${CMAKE_SYSTEM_NAME} ${CMAKE_SYSTEM_PROCESSOR}")
message(STATUS "VEX: Build type: ${CMAKE_BUILD_TYPE}")

# Include LLVM cmake utilities
include(AddLLVM)
include(HandleLLVMOptions)

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
    TransformUtils
    ScalarOpts
    InstCombine
    AggressiveInstCombine
    IPO
    Vectorize
    Coroutines
    CodeGen
    SelectionDAG
    AsmPrinter
    DebugInfoDWARF
    DebugInfoPDB
    DebugInfoMSF
    DebugInfoCodeView
)

# Map component names to actual LLVM library targets
llvm_map_components_to_libnames(VEX_LLVM_LIBS ${VEX_LLVM_COMPONENTS})

# ── Version header generation ─────────────────────────────────────────────────
configure_file(
    "${CMAKE_CURRENT_SOURCE_DIR}/cmake/VexVersion.cmake.in"
    "${CMAKE_CURRENT_BINARY_DIR}/include/vex/Core/VexVersion.h"
    @ONLY
)

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
    src/Parser/Parser.cpp
    src/Parser/ParseDecl.cpp
    src/Parser/ParseStmt.cpp
    src/Parser/ParseExpr.cpp
    src/Parser/ParseType.cpp
    src/Parser/ParsePattern.cpp
    src/Parser/ParseImport.cpp
)

set(VEX_AST_SOURCES
    src/AST/Visibility.cpp
    src/AST/ASTDumper.cpp
    src/AST/ASTVisitor.cpp
)

set(VEX_SEMA_SOURCES
    src/Sema/NameResolver.cpp
    src/Sema/TypeChecker.cpp
    src/Sema/BorrowChecker.cpp
    src/Sema/OverloadResolver.cpp
    src/Sema/Scope.cpp
    src/Sema/SymbolTable.cpp
    src/Sema/TypeInference.cpp
    src/Sema/VisibilityChecker.cpp
    src/Sema/ExhaustivenessChecker.cpp
    src/Sema/MROResolver.cpp
    src/Sema/MoveChecker.cpp
    src/Sema/ComptimeEvaluator.cpp
    src/Sema/ConstantFolder.cpp
    src/Sema/ControlFlowGraph.cpp
    src/Sema/LivenessAnalysis.cpp
    src/Sema/LifetimeAnalysis.cpp
    src/Sema/ImplResolver.cpp
    src/Sema/TraitSolver.cpp
    src/Sema/DecoratorProcessor.cpp
    src/Sema/DeriveExpander.cpp
    src/Sema/FormatStringChecker.cpp
    src/Sema/OverflowChecker.cpp
    src/Sema/OrphanChecker.cpp
    src/Sema/OwnershipRegion.cpp
    src/Sema/SemaContext.cpp
    src/Sema/Sema.cpp
    src/Sema/Symbol.cpp
    src/Sema/NameLookup.cpp
)

set(VEX_IR_SOURCES
    src/IR/IRGen.cpp
    src/IR/IRBuilder.cpp
    src/IR/IRBlock.cpp
    src/IR/IRFunction.cpp
    src/IR/IRModule.cpp
    src/IR/IRInstr.cpp
    src/IR/IRType.cpp
    src/IR/IRValue.cpp
    src/IR/IRVerifier.cpp
    src/IR/IRDumper.cpp
    src/IR/IRPassManager.cpp
    src/IR/LoweringContext.cpp
    src/IR/MatchLowering.cpp
    src/IR/AsyncLowering.cpp
    src/IR/ClosureLowering.cpp
    src/IR/DeferLowering.cpp
    src/IR/DropInserter.cpp
    src/IR/ErrorPropLowering.cpp
    src/IR/GeneratorLowering.cpp
    src/IR/MoveInserter.cpp
    src/IR/VexIR.cpp
)

set(VEX_CODEGEN_SOURCES
    src/CodeGen/CodeGen.cpp
    src/CodeGen/LLVMEmitter.cpp
    src/CodeGen/LLVMContext.cpp
    src/CodeGen/LLVMTypeMapper.cpp
    src/CodeGen/LLVMDebugInfo.cpp
    src/CodeGen/ExprEmitter.cpp
    src/CodeGen/StmtEmitter.cpp
    src/CodeGen/DeclEmitter.cpp
    src/CodeGen/TypeEmitter.cpp
    src/CodeGen/ConstantEmitter.cpp
    src/CodeGen/AtomicEmitter.cpp
    src/CodeGen/IntrinsicEmitter.cpp
    src/CodeGen/SIMDEmitter.cpp
    src/CodeGen/RTTIEmitter.cpp
    src/CodeGen/VTableEmitter.cpp
    src/CodeGen/Monomorphizer.cpp
    src/CodeGen/SpecRegistry.cpp
    src/CodeGen/StringPool.cpp
    src/CodeGen/TailCallOptimizer.cpp
    src/CodeGen/ABI.cpp
    src/CodeGen/SysVABI.cpp
    src/CodeGen/AAPCS64ABI.cpp
    src/CodeGen/Win64ABI.cpp
    src/CodeGen/WasmABI.cpp
)

set(VEX_MODULE_SOURCES
    src/Module/ImportResolver.cpp
    src/Module/ModuleLoader.cpp
    src/Module/ModuleCache.cpp
    src/Module/DependencyGraph.cpp
    src/Module/TopologicalSort.cpp
    src/Module/SelectiveImport.cpp
    src/Module/PackageResolver.cpp
    src/Module/TomlParser.cpp
    src/Module/VxlReader.cpp
    src/Module/VxlWriter.cpp
)

set(VEX_OPTIMIZER_SOURCES
    src/Optimizer/Optimizer.cpp
    src/Optimizer/PassPipeline.cpp
    src/Optimizer/DeadCodeElim.cpp
    src/Optimizer/EscapeAnalysis.cpp
    src/Optimizer/InlinerPolicy.cpp
    src/Optimizer/LTOManager.cpp
    src/Optimizer/PGOInstrumentation.cpp
    src/Optimizer/PollyIntegration.cpp
)

set(VEX_LINKER_SOURCES
    src/Linker/VexLinker.cpp
    src/Linker/SymbolResolver.cpp
    src/Linker/ExportTable.cpp
    src/Linker/LibraryManager.cpp
    src/Linker/ObjectEmitter.cpp
    src/Linker/SharedLibEmitter.cpp
    src/Linker/WasmEmitter.cpp
)

set(VEX_DRIVER_SOURCES
    src/Driver/main.cpp
    src/Driver/Driver.cpp
    src/Driver/Compilation.cpp
    src/Driver/CompilationUnit.cpp
    src/Driver/Options.cpp
    src/Driver/TargetInfo.cpp
    src/Driver/IncrementalCache.cpp
)

# All compiler sources (everything except main.cpp — for library)
set(VEX_LIB_SOURCES
    ${VEX_CORE_SOURCES}
    ${VEX_LEXER_SOURCES}
    ${VEX_PARSER_SOURCES}
    ${VEX_AST_SOURCES}
    ${VEX_SEMA_SOURCES}
    ${VEX_IR_SOURCES}
    ${VEX_CODEGEN_SOURCES}
    ${VEX_MODULE_SOURCES}
    ${VEX_OPTIMIZER_SOURCES}
    ${VEX_LINKER_SOURCES}
)

# ── Compiler library (vexlib) ─────────────────────────────────────────────────
add_library(vexlib STATIC ${VEX_LIB_SOURCES})

target_include_directories(vexlib
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/include
        ${CMAKE_CURRENT_BINARY_DIR}/include
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
        -Wno-unused-function
    )
elseif (MSVC)
    target_compile_options(vexlib PRIVATE /W4 /wd4100 /wd4505)
endif()

# ── Compiler executable (vexc) ────────────────────────────────────────────────
add_executable(vexc ${VEX_DRIVER_SOURCES})

target_link_libraries(vexc
    PRIVATE
        vexlib
        ${VEX_LLVM_LIBS}
)

target_include_directories(vexc
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/include
        ${CMAKE_CURRENT_BINARY_DIR}/include
        ${LLVM_INCLUDE_DIRS}
)

target_compile_features(vexc PRIVATE cxx_std_17)

# ── Runtime library ────────────────────────────────────────────────────────────
add_subdirectory(runtime)

# ── Tools ──────────────────────────────────────────────────────────────────────
if(VEX_BUILD_TOOLS)
    add_subdirectory(tools/vex-fmt)
    add_subdirectory(tools/vex-doc)
    add_subdirectory(tools/vex-lsp)
    add_subdirectory(tools/vpm)
endif()

# ── Tests ─────────────────────────────────────────────────────────────────────
if(VEX_BUILD_TESTS)
    enable_testing()
    add_subdirectory(tests)
endif()

# ── Polly integration (optional) ──────────────────────────────────────────────
if(VEX_ENABLE_POLLY)
    find_package(Polly QUIET)
    if(Polly_FOUND OR TARGET Polly)
        target_link_libraries(vexlib PUBLIC Polly LLVMPolly)
        target_compile_definitions(vexlib PUBLIC VEX_HAS_POLLY=1)
        message(STATUS "VEX: Polly integration enabled")
    else()
        message(WARNING "VEX_ENABLE_POLLY=ON but Polly was not found")
    endif()
endif()

# ── Install ───────────────────────────────────────────────────────────────────
include(VexInstall)

# ── Status messages ───────────────────────────────────────────────────────────
message(STATUS "VEX: Polly: ${VEX_ENABLE_POLLY}")
message(STATUS "VEX: Tests: ${VEX_BUILD_TESTS}")
message(STATUS "VEX: Tools: ${VEX_BUILD_TOOLS}")