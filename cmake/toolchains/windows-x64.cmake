# cmake/toolchains/windows-x64.cmake
# cmake/toolchains/windows-x64.cmake
set(CMAKE_SYSTEM_NAME Windows)
set(CMAKE_SYSTEM_PROCESSOR AMD64)

# MSVC is detected automatically on Windows
# For cross-compilation using clang-cl or MinGW, override here
if(NOT MSVC)
    find_program(CMAKE_C_COMPILER   NAMES clang-cl cl)
    find_program(CMAKE_CXX_COMPILER NAMES clang-cl cl)
endif()

set(CMAKE_C_FLAGS_RELEASE   "/O2 /DNDEBUG /MD")
set(CMAKE_CXX_FLAGS_RELEASE "/O2 /DNDEBUG /MD /EHsc")