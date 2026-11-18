# =============================================================================
# VEX Compiler — Production Build System
# =============================================================================
cmake_minimum_required(VERSION 3.20)
project(VEX VERSION 1.0.0 LANGUAGES C CXX)

# ── C++ standard ──────────────────────────────────────────────────────────────
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_POSITION_INDEPENDENT_CODE ON)

# ── CMake module path ─────────────────────────────────────────────────────────
list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")

# ── Options ───────────────────────────────────────────────────────────────────
option(VEX_BUILD_TESTS    "Build test suite"       OFF)
option(VEX_BUILD_TOOLS    "Build auxiliary tools"  OFF)
option(VEX_BUILD_RUNTIME  "Build VEX C runtime"    ON)
option(VEX_ENABLE_POLLY   "Enable Polly optimizer" ON)
option(VEX_BUILD_SHARED   "Build libvexcompiler as shared library" ON)
option(VEX_WARNINGS_AS_ERRORS "Treat warnings as errors" OFF)
option(VEX_USE_PREBUILT_LLVM  "Use prebuilt LLVM from llvm-prebuilt/" OFF)

# ── LLVM discovery ────────────────────────────────────────────────────────────
# Priority: 1) explicit LLVM_DIR, 2) prebuilt dir, 3) system LLVM
if(VEX_USE_PREBUILT_LLVM)
  set(_prebuilt "${CMAKE_CURRENT_SOURCE_DIR}/../llvm-prebuilt")
  if(EXISTS "${_prebuilt}/lib/cmake/llvm")
    set(LLVM_DIR "${_prebuilt}/lib/cmake/llvm")
  endif()
endif()

find_package(LLVM REQUIRED CONFIG)
message(STATUS "VEX: Using LLVM ${LLVM_VERSION} from ${LLVM_DIR}")

# Verify LLVM ≥ 15
if(LLVM_VERSION_MAJOR LESS 15)
  message(FATAL_ERROR "VEX requires LLVM 15 or newer (found ${LLVM_VERSION})")
endif()

include(AddLLVM)
include(HandleLLVMOptions)

# ── LLVM shared library strategy ──────────────────────────────────────────────
# Always prefer the monolithic libLLVM-<ver>.so for a small vexc binary.
# Fall back to per-component static libs only if the dylib isn't available.
# Use the monolithic LLVM shared library for small binary size.
# llvm_map_components_to_libnames() respects USE_SHARED/LLVM_LINK_LLVM_DYLIB.
set(_vex_llvm_components
  core support target targetparser
  X86CodeGen X86AsmParser X86Desc X86Info
  AArch64CodeGen AArch64AsmParser AArch64Desc AArch64Info
  WebAssemblyCodeGen WebAssemblyAsmParser WebAssemblyDesc WebAssemblyInfo
  RISCVCodeGen RISCVAsmParser RISCVDesc RISCVInfo
  Passes Analysis BitWriter BitReader IRReader Linker
  MC MCParser Object Option TransformUtils ScalarOpts InstCombine
  AggressiveInstCombine IPO Vectorize CodeGen SelectionDAG AsmPrinter
  DebugInfoDWARF LTO
)
llvm_map_components_to_libnames(VEX_LLVM_LIBS ${_vex_llvm_components})
# If the dylib is available use it instead (single .so, much smaller binary)
find_library(_llvm_dylib
  NAMES "LLVM-${LLVM_VERSION_MAJOR}" "LLVM-${LLVM_VERSION}" LLVM
  PATHS "${LLVM_LIBRARY_DIR}" NO_DEFAULT_PATH)
if(_llvm_dylib)
  set(VEX_LLVM_LIBS "${_llvm_dylib}")
  message(STATUS "VEX: Using monolithic LLVM dylib: ${_llvm_dylib}")
endif()

# ── LLD linker libraries (static — no shared liblld yet upstream) ─────────────
set(VEX_LLD_LIBS "")
foreach(_lld lldELF lldCOFF lldMachO lldWasm lldMinGW lldCommon)
  find_library(_lib${_lld} ${_lld}
    PATHS "${LLVM_LIBRARY_DIR}" NO_DEFAULT_PATH)
  if(_lib${_lld})
    list(APPEND VEX_LLD_LIBS "${_lib${_lld}}")
  endif()
endforeach()
if(NOT VEX_LLD_LIBS)
  message(WARNING "VEX: LLD static libs not found — linking will use system ld")
endif()
message(STATUS "VEX: LLD libs: ${VEX_LLD_LIBS}")

# ── Polly ─────────────────────────────────────────────────────────────────────
set(VEX_HAS_POLLY OFF)
if(VEX_ENABLE_POLLY)
  find_library(_polly_lib LLVMPolly
    PATHS "${LLVM_LIBRARY_DIR}" NO_DEFAULT_PATH)
  if(_polly_lib)
    set(VEX_HAS_POLLY ON)
    list(APPEND VEX_LLVM_LIBS "${_polly_lib}")
    message(STATUS "VEX: Polly enabled (${_polly_lib})")
  else()
    message(STATUS "VEX: Polly not found — optimizer will use standard LLVM passes")
  endif()
endif()

# ── Compiler-RT builtins ──────────────────────────────────────────────────────
set(VEX_RT_BUILTINS "")
set(_rt_path "${LLVM_LIBRARY_DIR}/clang/${LLVM_VERSION_MAJOR}/lib/linux")
find_library(_clang_rt_builtins
  NAMES "clang_rt.builtins-x86_64" "clang_rt.builtins"
  PATHS "${_rt_path}" NO_DEFAULT_PATH)
if(_clang_rt_builtins)
  set(VEX_RT_BUILTINS "${_clang_rt_builtins}")
  message(STATUS "VEX: compiler-rt builtins: ${_clang_rt_builtins}")
endif()

# ── Version header ────────────────────────────────────────────────────────────
configure_file(
  "${CMAKE_CURRENT_SOURCE_DIR}/cmake/VexVersion.cmake.in"
  "${CMAKE_CURRENT_BINARY_DIR}/include/vex/Core/VexVersion.h"
  @ONLY)

# ── RPATH configuration (for release binary finding its bundled libs) ─────────
# Embeds $ORIGIN/../lib so the installed vexc finds bundled libLLVM without
# needing the user to set LD_LIBRARY_PATH.
set(CMAKE_INSTALL_RPATH "$ORIGIN/../lib;$ORIGIN")
set(CMAKE_BUILD_WITH_INSTALL_RPATH ON)
set(CMAKE_INSTALL_RPATH_USE_LINK_PATH OFF)

# ── Source file lists ─────────────────────────────────────────────────────────

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
  src/AST/ASTNode.cpp
  src/AST/Decl.cpp
  src/AST/Expr.cpp
  src/AST/Stmt.cpp
  src/AST/Type.cpp
  src/AST/TypeRepr.cpp
  src/AST/Pattern.cpp
  src/AST/Attribute.cpp
  src/AST/Decorator.cpp
  src/AST/ASTContext.cpp
  src/AST/ASTSerializer.cpp
  src/AST/Visibility.cpp
  src/AST/ASTDumper.cpp
  src/AST/ASTVisitor.cpp
)

set(VEX_SEMA_SOURCES
  src/Sema/Symbol.cpp
  src/Sema/SymbolTable.cpp
  src/Sema/SemaContext.cpp
  src/Sema/NameResolver.cpp
  src/Sema/NameLookup.cpp
  src/Sema/TypeChecker.cpp
  src/Sema/TypeInference.cpp
  src/Sema/BorrowChecker.cpp
  src/Sema/MoveChecker.cpp
  src/Sema/LifetimeAnalysis.cpp
  src/Sema/LivenessAnalysis.cpp
  src/Sema/ControlFlowGraph.cpp
  src/Sema/OverloadResolver.cpp
  src/Sema/VisibilityChecker.cpp
  src/Sema/ExhaustivenessChecker.cpp
  src/Sema/MROResolver.cpp
  src/Sema/ComptimeEvaluator.cpp
  src/Sema/ConstantFolder.cpp
  src/Sema/DecoratorProcessor.cpp
  src/Sema/DeriveExpander.cpp
  src/Sema/FormatStringChecker.cpp
  src/Sema/OverflowChecker.cpp
  src/Sema/OrphanChecker.cpp
  src/Sema/OwnershipRegion.cpp
  src/Sema/ImplResolver.cpp
  src/Sema/TraitSolver.cpp
  src/Sema/Sema.cpp
)

set(VEX_IR_SOURCES
  src/IR/VexIR.cpp
  src/IR/IRType.cpp
  src/IR/IRBuilder.cpp
  src/IR/IRPassManager.cpp
  src/IR/IRVerifier.cpp
  src/IR/IRGen.cpp
  src/IR/LoweringContext.cpp
  src/IR/MatchLowering.cpp
  src/IR/AsyncLowering.cpp
  src/IR/ClosureLowering.cpp
  src/IR/DeferLowering.cpp
  src/IR/DropInserter.cpp
  src/IR/ErrorPropLowering.cpp
  src/IR/GeneratorLowering.cpp
  src/IR/MoveInserter.cpp
  src/IR/IRBlock.cpp
  src/IR/IRDumper.cpp
  src/IR/IRFunction.cpp
  src/IR/IRInstr.cpp
  src/IR/IRModule.cpp
  src/IR/IRValue.cpp
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

# ── All library sources (everything except main.cpp) ─────────────────────────
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
  src/Driver/Driver.cpp
  src/Driver/Compilation.cpp
  src/Driver/CompilationUnit.cpp
  src/Driver/Options.cpp
  src/Driver/TargetInfo.cpp
  src/Driver/IncrementalCache.cpp
)

# ── Compiler library ──────────────────────────────────────────────────────────
if(VEX_BUILD_SHARED)
  add_library(vexcompiler SHARED ${VEX_LIB_SOURCES})
  set_target_properties(vexcompiler PROPERTIES
    VERSION   ${PROJECT_VERSION}
    SOVERSION ${PROJECT_VERSION_MAJOR}
    OUTPUT_NAME vexcompiler
  )
else()
  add_library(vexcompiler STATIC ${VEX_LIB_SOURCES})
endif()

target_include_directories(vexcompiler
  PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<BUILD_INTERFACE:${CMAKE_CURRENT_BINARY_DIR}/include>
    $<INSTALL_INTERFACE:include>
    ${LLVM_INCLUDE_DIRS}
)

target_compile_definitions(vexcompiler
  PUBLIC ${LLVM_DEFINITIONS}
  PRIVATE
    VEX_VERSION_MAJOR=${PROJECT_VERSION_MAJOR}
    VEX_VERSION_MINOR=${PROJECT_VERSION_MINOR}
    VEX_VERSION_PATCH=${PROJECT_VERSION_PATCH}
    $<$<BOOL:${VEX_HAS_POLLY}>:VEX_HAS_POLLY=1>
)

target_link_libraries(vexcompiler
  PUBLIC
    ${VEX_LLVM_LIBS}
  PRIVATE
    ${VEX_LLD_LIBS}
)

target_compile_features(vexcompiler PUBLIC cxx_std_17)

if(CMAKE_CXX_COMPILER_ID MATCHES "Clang|GNU")
  target_compile_options(vexcompiler PRIVATE
    -Wall -Wextra -Wshadow
    -Wno-unused-parameter -Wno-unused-function -Wno-unused-variable
    -Wno-comment
    $<$<CONFIG:Release>:-O3 -DNDEBUG>
    $<$<CONFIG:Debug>:-O0 -g3>
  )
  if(VEX_WARNINGS_AS_ERRORS)
    target_compile_options(vexcompiler PRIVATE -Werror)
  endif()
endif()

# ── vexc executable (small binary) ───────────────────────────────────────────
add_executable(vexc src/Driver/main.cpp)

target_link_libraries(vexc PRIVATE vexcompiler)

target_include_directories(vexc PRIVATE
  ${CMAKE_CURRENT_SOURCE_DIR}/include
  ${CMAKE_CURRENT_BINARY_DIR}/include
  ${LLVM_INCLUDE_DIRS}
)

target_compile_features(vexc PRIVATE cxx_std_17)

if(CMAKE_CXX_COMPILER_ID MATCHES "Clang|GNU")
  target_compile_options(vexc PRIVATE -Wno-unused-parameter)
endif()

# ── Runtime library ───────────────────────────────────────────────────────────
if(VEX_BUILD_RUNTIME)
  add_subdirectory(runtime)
endif()

# ── Tools ─────────────────────────────────────────────────────────────────────
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

# ── Install rules ─────────────────────────────────────────────────────────────
include(GNUInstallDirs)
include(cmake/VexInstall.cmake)

# ── Summary ───────────────────────────────────────────────────────────────────
message(STATUS "")
message(STATUS "  ╔══════════════════════════════════════╗")
message(STATUS "  ║  VEX Compiler v${PROJECT_VERSION} build config  ║")
message(STATUS "  ╠══════════════════════════════════════╣")
message(STATUS "  ║  LLVM:       ${LLVM_VERSION}")
message(STATUS "  ║  LLD:        ${VEX_LLD_LIBS}")
message(STATUS "  ║  Polly:      ${VEX_HAS_POLLY}")
message(STATUS "  ║  Library:    $<IF:$<BOOL:${VEX_BUILD_SHARED}>,shared,static>")
message(STATUS "  ║  Runtime:    ${VEX_BUILD_RUNTIME}")
message(STATUS "  ║  Tests:      ${VEX_BUILD_TESTS}")
message(STATUS "  ║  Build type: ${CMAKE_BUILD_TYPE}")
message(STATUS "  ╚══════════════════════════════════════╝")
message(STATUS "")
