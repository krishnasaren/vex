# cmake/VexTestHelpers.cmake
include(GoogleTest)

function(vex_register_test target)
    gtest_discover_tests(${target}
        WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
        PROPERTIES
            TIMEOUT 60
            LABELS "unit"
    )
endfunction()

function(vex_register_integration_test target)
    add_test(NAME ${target}
        COMMAND $<TARGET_FILE:${target}>
        WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
    )
    set_tests_properties(${target} PROPERTIES
        TIMEOUT 120
        LABELS "integration"
    )
endfunction()