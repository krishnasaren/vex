# tests/CMakeLists.txt
cmake_minimum_required(VERSION 3.20)

include(VexTestHelpers)
include(GoogleTest)

# ── Unit tests ────────────────────────────────────────────────────────────────
add_subdirectory(unit)

# ── Integration tests ─────────────────────────────────────────────────────────
add_subdirectory(integration)

# ── Benchmarks ────────────────────────────────────────────────────────────────
if(VEX_BUILD_BENCHMARKS)
    add_subdirectory(bench)
endif()

# ── Fuzz targets ──────────────────────────────────────────────────────────────
if(VEX_ENABLE_FUZZING)
    add_subdirectory(fuzz)
endif()