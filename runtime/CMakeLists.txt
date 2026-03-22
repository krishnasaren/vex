cmake_minimum_required(VERSION 3.20)

if(NOT VEX_BUILD_RUNTIME)
    return()
endif()

# Runtime C sources
set(VEX_RT_SOURCES
    src/alloc.c
    src/arc.c
    src/arena.c
    src/panic.c
    src/io.c
    src/init.c
    src/signal.c
    src/stackguard.c
    src/sync.c
    src/thread.c
    src/tls.c
    src/typeid.c
    src/waker.c
    src/workstealing.c
    src/executor.c
    src/async.c
    src/channel.c
)

# Platform-specific sources
if(CMAKE_SYSTEM_NAME STREQUAL "Linux")
    list(APPEND VEX_RT_SOURCES src/platform/linux.c)
elseif(CMAKE_SYSTEM_NAME STREQUAL "Darwin")
    list(APPEND VEX_RT_SOURCES src/platform/macos.c)
elseif(CMAKE_SYSTEM_NAME STREQUAL "Windows")
    list(APPEND VEX_RT_SOURCES src/platform/windows.c)
elseif(CMAKE_SYSTEM_NAME STREQUAL "Generic")
    list(APPEND VEX_RT_SOURCES src/platform/wasm.c)
endif()

add_library(vex_rt STATIC ${VEX_RT_SOURCES})

target_include_directories(vex_rt
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/include
)

target_compile_features(vex_rt PUBLIC c_std_11)

if(CMAKE_C_COMPILER_ID MATCHES "Clang|GNU")
    target_compile_options(vex_rt PRIVATE
        -Wall -Wextra -Wno-unused-parameter
        -fno-strict-aliasing
    )
endif()

# Link pthread on Linux/macOS
if(CMAKE_SYSTEM_NAME MATCHES "Linux|Darwin")
    find_package(Threads REQUIRED)
    target_link_libraries(vex_rt PUBLIC Threads::Threads)
endif()

# Install
install(TARGETS vex_rt
    ARCHIVE DESTINATION lib
    COMPONENT VexRuntime
)
install(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/include/vex_rt
    DESTINATION include
    COMPONENT VexRuntime
    FILES_MATCHING PATTERN "*.h"
)
