# cmake/FindLLVM.cmake
# Thin wrapper around LLVM's own CMake config.
# Usage: find_package(LLVM REQUIRED CONFIG) is preferred.
# This file exists as a fallback for non-standard LLVM installations.

if(DEFINED LLVM_DIR AND EXISTS "${LLVM_DIR}/LLVMConfig.cmake")
    include("${LLVM_DIR}/LLVMConfig.cmake")
    set(LLVM_FOUND TRUE)
    return()
endif()

# Common LLVM installation paths
set(_llvm_search_paths
    /usr/lib/llvm-17/lib/cmake/llvm
    /usr/lib/llvm-17/cmake
    /usr/local/lib/cmake/llvm
    /usr/local/lib/llvm17/cmake
    /opt/llvm/lib/cmake/llvm
    /opt/homebrew/opt/llvm/lib/cmake/llvm   # macOS Homebrew
    "C:/Program Files/LLVM/lib/cmake/llvm"  # Windows
)

foreach(path ${_llvm_search_paths})
    if(EXISTS "${path}/LLVMConfig.cmake")
        set(LLVM_DIR "${path}" CACHE PATH "LLVM cmake directory")
        include("${path}/LLVMConfig.cmake")
        set(LLVM_FOUND TRUE)
        break()
    endif()
endforeach()

if(NOT LLVM_FOUND)
    if(FindLLVM_FIND_REQUIRED)
        message(FATAL_ERROR
            "LLVM not found. Install LLVM 17 and set -DLLVM_DIR=/path/to/llvm/lib/cmake/llvm\n"
            "On Ubuntu/Debian: sudo apt install llvm-17-dev\n"
            "On macOS: brew install llvm@17")
    endif()
    set(LLVM_FOUND FALSE)
endif()

mark_as_advanced(LLVM_DIR)