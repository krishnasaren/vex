# cmake/VexInstall.cmake

include(GNUInstallDirs)
include(CMakePackageConfigHelpers)

# Install binaries
install(TARGETS vexc vexlib
    EXPORT VexTargets
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
    ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    COMPONENT VexDev
)

# Export targets
install(EXPORT VexTargets
    FILE VexTargets.cmake
    NAMESPACE Vex::
    DESTINATION "${VEX_CMAKE_CONFIG_DIR}"
)

# Install headers
install(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/include/vex
    DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
    COMPONENT VexDev
    FILES_MATCHING PATTERN "*.h"
)

# Install cmake config
set(VEX_CMAKE_CONFIG_DIR "${CMAKE_INSTALL_LIBDIR}/cmake/vex")

configure_package_config_file(
    "${CMAKE_CURRENT_SOURCE_DIR}/cmake/VexConfig.cmake.in"
    "${CMAKE_CURRENT_BINARY_DIR}/VexConfig.cmake"
    INSTALL_DESTINATION "${VEX_CMAKE_CONFIG_DIR}"
)

write_basic_package_version_file(
    "${CMAKE_CURRENT_BINARY_DIR}/VexConfigVersion.cmake"
    VERSION ${PROJECT_VERSION}
    COMPATIBILITY SameMajorVersion
)

install(FILES
    "${CMAKE_CURRENT_BINARY_DIR}/VexConfig.cmake"
    "${CMAKE_CURRENT_BINARY_DIR}/VexConfigVersion.cmake"
    DESTINATION "${VEX_CMAKE_CONFIG_DIR}"
    COMPONENT VexDev
)

# Install stdlib
if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/stdlib")
    install(DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/stdlib/"
        DESTINATION "${CMAKE_INSTALL_DATADIR}/vex/lib"
        COMPONENT VexStdlib
    )
endif()

# CPack packaging
set(CPACK_PACKAGE_NAME "VEXCompiler")
set(CPACK_PACKAGE_VENDOR "VEX Language")
set(CPACK_PACKAGE_DESCRIPTION_SUMMARY "VEX Compiler - Compiled, Native, No GC")
set(CPACK_PACKAGE_VERSION ${PROJECT_VERSION})
set(CPACK_PACKAGE_VERSION_MAJOR ${PROJECT_VERSION_MAJOR})
set(CPACK_PACKAGE_VERSION_MINOR ${PROJECT_VERSION_MINOR})
set(CPACK_PACKAGE_VERSION_PATCH ${PROJECT_VERSION_PATCH})
set(CPACK_RESOURCE_FILE_LICENSE "${CMAKE_CURRENT_SOURCE_DIR}/LICENSE")
set(CPACK_RESOURCE_FILE_README  "${CMAKE_CURRENT_SOURCE_DIR}/README.md")

include(CPack)