# cmake/FindPolly.cmake
# cmake/FindPolly.cmake
# Find Polly loop optimizer (part of LLVM project)

find_library(POLLY_LIBRARY
    NAMES Polly
    HINTS
        ${LLVM_LIBRARY_DIR}
        ${LLVM_LIBRARY_DIRS}
        /usr/lib/llvm-17/lib
        /usr/local/lib
        /opt/homebrew/opt/llvm/lib
)

find_library(LLVM_POLLY_LIBRARY
    NAMES LLVMPolly
    HINTS
        ${LLVM_LIBRARY_DIR}
        ${LLVM_LIBRARY_DIRS}
        /usr/lib/llvm-17/lib
        /usr/local/lib
        /opt/homebrew/opt/llvm/lib
)

find_path(POLLY_INCLUDE_DIR
    NAMES polly/LinkAllPasses.h
    HINTS
        ${LLVM_INCLUDE_DIRS}
        /usr/lib/llvm-17/include
        /opt/homebrew/opt/llvm/include
)

if(POLLY_LIBRARY AND LLVM_POLLY_LIBRARY AND POLLY_INCLUDE_DIR)
    set(Polly_FOUND TRUE)
    set(POLLY_LIBRARIES ${POLLY_LIBRARY} ${LLVM_POLLY_LIBRARY})
    set(POLLY_INCLUDE_DIRS ${POLLY_INCLUDE_DIR})
    message(STATUS "VEX: Polly found at ${POLLY_LIBRARY}")
else()
    set(Polly_FOUND FALSE)
    if(FindPolly_FIND_REQUIRED)
        message(FATAL_ERROR "Polly not found. Install LLVM with Polly support.")
    endif()
endif()

mark_as_advanced(POLLY_LIBRARY LLVM_POLLY_LIBRARY POLLY_INCLUDE_DIR)