# cmake/VexInstall.cmake — Install rules and release packaging
include(GNUInstallDirs)
include(CMakePackageConfigHelpers)

set(VEX_INSTALL_PREFIX "VEX-${PROJECT_VERSION}-linux-x86_64")

# ── Binaries ──────────────────────────────────────────────────────────────────
install(TARGETS vexc
  RUNTIME DESTINATION "${VEX_INSTALL_PREFIX}/bin"
)

install(TARGETS vexcompiler
  LIBRARY DESTINATION "${VEX_INSTALL_PREFIX}/lib"
  ARCHIVE DESTINATION "${VEX_INSTALL_PREFIX}/lib"
)

if(VEX_BUILD_RUNTIME)
  install(TARGETS vex_rt
    LIBRARY DESTINATION "${VEX_INSTALL_PREFIX}/lib"
    ARCHIVE DESTINATION "${VEX_INSTALL_PREFIX}/lib"
  )
endif()

# ── LLVM shared library bundling ──────────────────────────────────────────────
# Bundle libLLVM-18.so so the release is self-contained
find_library(_llvm_so
  NAMES "LLVM-${LLVM_VERSION_MAJOR}" "LLVM-${LLVM_VERSION}" LLVM
  PATHS "${LLVM_LIBRARY_DIR}" NO_DEFAULT_PATH)
if(_llvm_so)
  install(FILES "${_llvm_so}"
    DESTINATION "${VEX_INSTALL_PREFIX}/lib"
    RENAME "libLLVM-${LLVM_VERSION_MAJOR}.so.${LLVM_VERSION}"
  )
  install(CODE "
    execute_process(COMMAND ${CMAKE_COMMAND} -E create_symlink
      libLLVM-${LLVM_VERSION_MAJOR}.so.${LLVM_VERSION}
      ${CMAKE_INSTALL_PREFIX}/${VEX_INSTALL_PREFIX}/lib/libLLVM-${LLVM_VERSION_MAJOR}.so)
  ")
endif()

# ── Polly shared library bundling ─────────────────────────────────────────────
if(VEX_HAS_POLLY AND _polly_lib)
  install(FILES "${_polly_lib}"
    DESTINATION "${VEX_INSTALL_PREFIX}/lib"
  )
endif()

# ── Compiler-RT builtins ──────────────────────────────────────────────────────
if(VEX_RT_BUILTINS)
  install(FILES "${VEX_RT_BUILTINS}"
    DESTINATION "${VEX_INSTALL_PREFIX}/runtime/builtins"
  )
endif()

# ── Standard library (VEX source files) ──────────────────────────────────────
install(DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/stdlib/"
  DESTINATION "${VEX_INSTALL_PREFIX}/std"
  FILES_MATCHING PATTERN "*.vex"
)

# ── Headers ───────────────────────────────────────────────────────────────────
install(DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/include/vex"
  DESTINATION "${VEX_INSTALL_PREFIX}/include"
)

# ── Docs ──────────────────────────────────────────────────────────────────────
install(DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/docs/"
  DESTINATION "${VEX_INSTALL_PREFIX}/docs"
  OPTIONAL
)

# ── Examples ──────────────────────────────────────────────────────────────────
install(DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/examples/"
  DESTINATION "${VEX_INSTALL_PREFIX}/examples"
  OPTIONAL
)

# ── Metadata files ────────────────────────────────────────────────────────────
install(FILES
  "${CMAKE_CURRENT_SOURCE_DIR}/README.md"
  "${CMAKE_CURRENT_SOURCE_DIR}/LICENSE"
  "${CMAKE_CURRENT_SOURCE_DIR}/CHANGELOG.md"
  DESTINATION "${VEX_INSTALL_PREFIX}"
)

# VERSION file
install(CODE "
  file(WRITE \"${CMAKE_INSTALL_PREFIX}/${VEX_INSTALL_PREFIX}/VERSION\"
       \"VEX ${PROJECT_VERSION}\nLLVM ${LLVM_VERSION}\nBuild: Release\n\")
")

# BUILD-INFO file
install(CODE "
  file(WRITE \"${CMAKE_INSTALL_PREFIX}/${VEX_INSTALL_PREFIX}/BUILD-INFO\"
       \"VEX Compiler Build Information\n\"
       \"Version:     ${PROJECT_VERSION}\n\"
       \"LLVM:        ${LLVM_VERSION}\n\"
       \"Build type:  Release\n\"
       \"Host:        x86_64-linux-gnu\n\"
       \"Date:        \$ENV{SOURCE_DATE_EPOCH}\n\")
")

# ── CPack for tarball creation ────────────────────────────────────────────────
set(CPACK_PACKAGE_NAME        "VEX")
set(CPACK_PACKAGE_VERSION     "${PROJECT_VERSION}")
set(CPACK_PACKAGE_DESCRIPTION "VEX Programming Language Compiler")
set(CPACK_GENERATOR           "TGZ")
set(CPACK_PACKAGE_FILE_NAME   "${VEX_INSTALL_PREFIX}")
include(CPack)
