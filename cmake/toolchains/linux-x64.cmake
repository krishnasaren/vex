# cmake/toolchains/linux-x64.cmake
set(CMAKE_SYSTEM_NAME Linux)
set(CMAKE_SYSTEM_PROCESSOR x86_64)

# Use system compiler by default; override with CC/CXX env vars
if(NOT CMAKE_C_COMPILER)
    find_program(CMAKE_C_COMPILER NAMES clang-17 clang gcc cc)
endif()
if(NOT CMAKE_CXX_COMPILER)
    find_program(CMAKE_CXX_COMPILER NAMES clang++-17 clang++ g++ c++)
endif()

set(CMAKE_C_COMPILER_TARGET   x86_64-linux-gnu)
set(CMAKE_CXX_COMPILER_TARGET x86_64-linux-gnu)

# Native tuning
set(CMAKE_C_FLAGS_RELEASE   "-O2 -DNDEBUG -march=x86-64 -mtune=generic")
set(CMAKE_CXX_FLAGS_RELEASE "-O2 -DNDEBUG -march=x86-64 -mtune=generic")