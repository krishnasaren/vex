# VEX Runtime Library
project(vex_rt C CXX)

set(VEX_RUNTIME_SOURCES
  src/vex_alloc.c
  src/vex_panic.c
  src/vex_io.c
  src/vex_string.c
  src/vex_array.c
  src/vex_map.c
  src/vex_result.c
  src/vex_option.c
  src/vex_thread.c
  src/vex_time.c
  src/vex_math.c
  src/vex_process.c
  src/vex_fs.c
  src/vex_net.c
  src/vex_init.c
)

add_library(vex_rt SHARED ${VEX_RUNTIME_SOURCES})

target_include_directories(vex_rt
  PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:include>
)

set_target_properties(vex_rt PROPERTIES
  VERSION   1.0.0
  SOVERSION 1
  OUTPUT_NAME vexruntime
  C_STANDARD 11
  C_STANDARD_REQUIRED ON
)

target_compile_options(vex_rt PRIVATE
  -Wall -Wextra -Wno-unused-parameter
  $<$<CONFIG:Release>:-O2 -DNDEBUG>
  $<$<CONFIG:Debug>:-O0 -g3 -DVEX_DEBUG>
)

# Link pthread and dl for threading / dynamic loading
target_link_libraries(vex_rt PRIVATE pthread dl m)

install(TARGETS vex_rt
  LIBRARY DESTINATION "VEX-1.0.0-linux-x86_64/lib"
  ARCHIVE DESTINATION "VEX-1.0.0-linux-x86_64/lib"
)

install(DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/include/"
  DESTINATION "VEX-1.0.0-linux-x86_64/include/vex/rt"
)
