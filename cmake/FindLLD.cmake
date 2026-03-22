# cmake/FindLLD.cmake
# cmake/FindLLD.cmake
# Find LLD linker libraries (part of LLVM project)

if(NOT LLVM_FOUND)
    find_package(LLVM QUIET CONFIG)
endif()

# LLD libraries to find
set(_lld_libs
    lldELF
    lldCOFF
    lldMachO
    lldWasm
    lldCommon
)

set(LLD_LIBRARIES)
set(LLD_FOUND TRUE)

foreach(lib ${_lld_libs})
    find_library(LLD_${lib}_LIBRARY
        NAMES ${lib}
        HINTS
            ${LLVM_LIBRARY_DIR}
            ${LLVM_LIBRARY_DIRS}
            /usr/lib/llvm-17/lib
            /usr/local/lib
            /opt/homebrew/opt/llvm/lib
    )
    if(LLD_${lib}_LIBRARY)
        list(APPEND LLD_LIBRARIES ${LLD_${lib}_LIBRARY})
    else()
        set(LLD_FOUND FALSE)
        if(FindLLD_FIND_REQUIRED)
            message(FATAL_ERROR "LLD library ${lib} not found")
        endif()
    endif()
endforeach()

# Find LLD include directory
find_path(LLD_INCLUDE_DIR
    NAMES lld/Common/Driver.h
    HINTS
        ${LLVM_INCLUDE_DIRS}
        /usr/lib/llvm-17/include
        /usr/local/include
        /opt/homebrew/opt/llvm/include
)

if(LLD_INCLUDE_DIR)
    set(LLD_INCLUDE_DIRS ${LLD_INCLUDE_DIR})
else()
    set(LLD_FOUND FALSE)
endif()

if(LLD_FOUND)
    message(STATUS "VEX: LLD found — native linking enabled")
else()
    message(STATUS "VEX: LLD not found — using system linker")
endif()

mark_as_advanced(LLD_INCLUDE_DIR ${_lld_libs})