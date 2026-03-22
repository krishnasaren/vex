# tests/unit/CMakeLists.txt
cmake_minimum_required(VERSION 3.20)

# ── Core tests ────────────────────────────────────────────────────────────────
add_executable(VexCoreTests
    Core/DiagnosticTest.cpp
    Core/SourceLocationTest.cpp
)
target_link_libraries(VexCoreTests PRIVATE vexlib GTest::gtest GTest::gtest_main)
target_include_directories(VexCoreTests PRIVATE
    ${CMAKE_SOURCE_DIR}/include
    ${LLVM_INCLUDE_DIRS}
)
gtest_discover_tests(VexCoreTests)

# ── Lexer tests ────────────────────────────────────────────────────────────────
add_executable(VexLexerTests
    Lexer/LexerTest.cpp
    Lexer/LiteralTest.cpp
    Lexer/TokenTest.cpp
    Lexer/UnicodeTest.cpp
)
target_link_libraries(VexLexerTests PRIVATE vexlib GTest::gtest GTest::gtest_main)
target_include_directories(VexLexerTests PRIVATE
    ${CMAKE_SOURCE_DIR}/include
    ${LLVM_INCLUDE_DIRS}
)
gtest_discover_tests(VexLexerTests)

# ── Parser tests ───────────────────────────────────────────────────────────────
add_executable(VexParserTests
    Parser/DeclParseTest.cpp
    Parser/ExprParseTest.cpp
    Parser/StmtParseTest.cpp
    Parser/TypeParseTest.cpp
    Parser/PatternParseTest.cpp
    Parser/ImportParseTest.cpp
    Parser/RecoveryTest.cpp
)
target_link_libraries(VexParserTests PRIVATE vexlib GTest::gtest GTest::gtest_main)
target_include_directories(VexParserTests PRIVATE
    ${CMAKE_SOURCE_DIR}/include
    ${LLVM_INCLUDE_DIRS}
)
gtest_discover_tests(VexParserTests)

# ── Sema tests ─────────────────────────────────────────────────────────────────
add_executable(VexSemaTests
    Sema/TypeCheckerTest.cpp
    Sema/TypeInferenceTest.cpp
    Sema/NameResolutionTest.cpp
    Sema/BorrowCheckerTest.cpp
    Sema/MoveCheckerTest.cpp
    Sema/VisibilityTest.cpp
    Sema/ExhaustivenessTest.cpp
    Sema/ImportTest.cpp
    Sema/SelectiveImportTest.cpp
    Sema/OverloadTest.cpp
    Sema/MROTest.cpp
    Sema/DeriveTest.cpp
    Sema/ComptimeTest.cpp
    Sema/LifetimeTest.cpp
    Sema/ModKeywordTest.cpp
    Sema/OrphanChecker.cpp
)
target_link_libraries(VexSemaTests PRIVATE vexlib GTest::gtest GTest::gtest_main)
target_include_directories(VexSemaTests PRIVATE
    ${CMAKE_SOURCE_DIR}/include
    ${LLVM_INCLUDE_DIRS}
)
gtest_discover_tests(VexSemaTests)