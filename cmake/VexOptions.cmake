# cmake/VexOptions.cmake
# All user-configurable VEX build options

option(VEX_ENABLE_POLLY     "Enable Polly loop optimizer integration"    OFF)
option(VEX_BUILD_TESTS      "Build unit and integration tests"            ON)
option(VEX_BUILD_TOOLS      "Build vex-fmt, vex-doc, vex-lsp, vpm tools" ON)
option(VEX_BUILD_RUNTIME    "Build the VEX runtime library"               ON)
option(VEX_ENABLE_LTO       "Enable Link-Time Optimization in release"    OFF)
option(VEX_ENABLE_ASAN      "Enable AddressSanitizer"                     OFF)
option(VEX_ENABLE_UBSAN     "Enable UndefinedBehaviorSanitizer"           OFF)
option(VEX_ENABLE_TSAN      "Enable ThreadSanitizer"                      OFF)
option(VEX_ENABLE_MSAN      "Enable MemorySanitizer"                      OFF)
option(VEX_ENABLE_COVERAGE  "Enable code coverage instrumentation"        OFF)
option(VEX_WARNINGS_AS_ERRORS "Treat compiler warnings as errors"         OFF)
option(VEX_ENABLE_ASSERTIONS "Enable runtime assertions (default: Debug)" ON)

# Sanitizer setup
include(VexSanitizers)

# LTO
if(VEX_ENABLE_LTO AND CMAKE_BUILD_TYPE MATCHES "Release|RelWithDebInfo")
    include(CheckIPOSupported)
    check_ipo_supported(RESULT ipo_supported OUTPUT ipo_err)
    if(ipo_supported)
        set(CMAKE_INTERPROCEDURAL_OPTIMIZATION TRUE)
        message(STATUS "VEX: LTO enabled")
    else()
        message(WARNING "VEX_ENABLE_LTO=ON but LTO not supported: ${ipo_err}")
    endif()
endif()

# Assertions
if(NOT VEX_ENABLE_ASSERTIONS OR CMAKE_BUILD_TYPE MATCHES "Release|MinSizeRel")
    target_compile_definitions(vexlib PUBLIC VEX_DISABLE_ASSERTIONS=1)
endif()