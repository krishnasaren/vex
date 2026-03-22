# cmake/toolchains/wasm.cmake
set(CMAKE_SYSTEM_NAME      Generic)
set(CMAKE_SYSTEM_PROCESSOR wasm32)

find_program(EMCC emcc)
find_program(EMXX em++)

if(EMCC AND EMXX)
    set(CMAKE_C_COMPILER   ${EMCC})
    set(CMAKE_CXX_COMPILER ${EMXX})
    set(CMAKE_AR           emar)
    set(CMAKE_RANLIB       emranlib)
    message(STATUS "VEX: Using Emscripten for WASM target")
else()
    find_program(CMAKE_C_COMPILER   NAMES clang-17 clang)
    find_program(CMAKE_CXX_COMPILER NAMES clang++-17 clang++)
    set(CMAKE_C_COMPILER_TARGET   wasm32-wasi)
    set(CMAKE_CXX_COMPILER_TARGET wasm32-wasi)
    message(STATUS "VEX: Using Clang WASM target (no Emscripten found)")
endif()

set(CMAKE_EXECUTABLE_SUFFIX ".wasm")
set(CMAKE_C_FLAGS_RELEASE   "-O2 -DNDEBUG")
set(CMAKE_CXX_FLAGS_RELEASE "-O2 -DNDEBUG")