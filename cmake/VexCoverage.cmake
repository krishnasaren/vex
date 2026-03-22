# cmake/VexCoverage.cmake
# cmake/VexCoverage.cmake
# Code coverage report generation

if(NOT VEX_ENABLE_COVERAGE)
    return()
endif()

find_program(LCOV_PATH lcov)
find_program(GENHTML_PATH genhtml)
find_program(LLVM_COV_PATH NAMES llvm-cov-17 llvm-cov)
find_program(LLVM_PROFDATA_PATH NAMES llvm-profdata-17 llvm-profdata)

# Target: coverage_report
add_custom_target(coverage_report
    COMMAND ${LCOV_PATH} --capture
        --directory ${CMAKE_BINARY_DIR}
        --output-file coverage.info
    COMMAND ${LCOV_PATH} --remove coverage.info
        '/usr/*' '*/tests/*' '*/build/*'
        --output-file coverage_filtered.info
    COMMAND ${GENHTML_PATH} coverage_filtered.info
        --output-directory coverage_html
        --title "VEX Compiler Coverage"
    WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
    COMMENT "Generating code coverage report"
)

# LLVM-style coverage with llvm-cov
if(LLVM_COV_PATH AND LLVM_PROFDATA_PATH)
    add_custom_target(llvm_coverage
        COMMAND ${CMAKE_COMMAND} -E env LLVM_PROFILE_FILE=vex.profraw
            $<TARGET_FILE:vexc> --version
        COMMAND ${LLVM_PROFDATA_PATH} merge -sparse vex.profraw -o vex.profdata
        COMMAND ${LLVM_COV_PATH} show $<TARGET_FILE:vexc>
            -instr-profile=vex.profdata
            -format=html
            -output-dir=llvm_coverage_html
        WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
        COMMENT "Generating LLVM coverage report"
    )
endif()