# cmake/toolchains/linux-arm64.cmake
set(CMAKE_SYSTEM_NAME Linux)
set(CMAKE_SYSTEM_PROCESSOR aarch64)

find_program(CMAKE_C_COMPILER   NAMES aarch64-linux-gnu-gcc clang-17 gcc)
find_program(CMAKE_CXX_COMPILER NAMES aarch64-linux-gnu-g++ clang++-17 g++)

set(CMAKE_C_COMPILER_TARGET   aarch64-linux-gnu)
set(CMAKE_CXX_COMPILER_TARGET aarch64-linux-gnu)

set(CMAKE_C_FLAGS_RELEASE   "-O2 -DNDEBUG -march=armv8-a")
set(CMAKE_CXX_FLAGS_RELEASE "-O2 -DNDEBUG -march=armv8-a")