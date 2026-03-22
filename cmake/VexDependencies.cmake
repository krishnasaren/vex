# cmake/VexDependencies.cmake
# Handles finding and configuring all external dependencies

# ── LLVM ─────────────────────────────────────────────────────────────────────
# LLVM is required. Set LLVM_DIR to your LLVM cmake config directory.
# Example: cmake -DLLVM_DIR=/path/to/llvm/lib/cmake/llvm ..

find_package(LLVM REQUIRED CONFIG)
if(NOT LLVM_FOUND)
    message(FATAL_ERROR
        "LLVM not found. Please set LLVM_DIR to the LLVM cmake config directory.\n"
        "Example: cmake -DLLVM_DIR=/usr/lib/llvm-17/lib/cmake/llvm ..")
endif()

message(STATUS "Found LLVM ${LLVM_VERSION} at ${LLVM_DIR}")

# ── GoogleTest (for unit tests) ────────────────────────────────────────────────
if(VEX_BUILD_TESTS)
    find_package(GTest QUIET)
    if(NOT GTest_FOUND)
        # Try to fetch from source
        include(FetchContent)
        FetchContent_Declare(
            googletest
            GIT_REPOSITORY https://github.com/google/googletest.git
            GIT_TAG        v1.14.0
        )
        # Prevent GoogleTest from overriding our compiler/linker settings
        set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
        FetchContent_MakeAvailable(googletest)
        message(STATUS "VEX: GoogleTest fetched from source")
    else()
        message(STATUS "VEX: GoogleTest found: ${GTest_VERSION}")
    endif()
endif()

# ── zlib (optional, for compressed debug info) ────────────────────────────────
find_package(ZLIB QUIET)
if(ZLIB_FOUND)
    message(STATUS "VEX: zlib found — compressed debug info supported")
    target_link_libraries(vexlib PUBLIC ZLIB::ZLIB)
    target_compile_definitions(vexlib PUBLIC VEX_HAS_ZLIB=1)
endif()

# ── libffi (optional, for JIT/FFI support) ────────────────────────────────────
find_package(LibFFI QUIET)
if(LibFFI_FOUND)
    message(STATUS "VEX: libffi found — FFI support enabled")
endif()