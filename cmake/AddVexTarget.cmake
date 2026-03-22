# cmake/AddVexTarget.cmake
# cmake/AddVexTarget.cmake
# Macros for creating VEX compiler build targets consistently

include_guard(GLOBAL)

# vex_compiler_target(name SOURCES src1.cpp src2.cpp [DEPS dep1 dep2])
# Creates a library target with all standard VEX compiler settings applied.
macro(vex_compiler_target name)
    cmake_parse_arguments(VCT "" "" "SOURCES;DEPS;DEFS" ${ARGN})

    add_library(${name} STATIC ${VCT_SOURCES})

    target_include_directories(${name}
        PUBLIC
            ${CMAKE_SOURCE_DIR}/include
            ${CMAKE_BINARY_DIR}/include
            ${LLVM_INCLUDE_DIRS}
    )

    target_compile_definitions(${name}
        PUBLIC ${LLVM_DEFINITIONS} ${VCT_DEFS}
    )

    target_compile_features(${name} PUBLIC cxx_std_17)

    if(VCT_DEPS)
        target_link_libraries(${name} PUBLIC ${VCT_DEPS})
    endif()

    if(CMAKE_CXX_COMPILER_ID MATCHES "Clang|GNU")
        target_compile_options(${name} PRIVATE
            -Wall -Wextra -Wshadow -Wno-unused-parameter -Wno-unused-function
        )
        if(VEX_WARNINGS_AS_ERRORS)
            target_compile_options(${name} PRIVATE -Werror)
        endif()
    elseif(MSVC)
        target_compile_options(${name} PRIVATE /W4 /wd4100 /wd4505)
        if(VEX_WARNINGS_AS_ERRORS)
            target_compile_options(${name} PRIVATE /WX)
        endif()
    endif()
endmacro()