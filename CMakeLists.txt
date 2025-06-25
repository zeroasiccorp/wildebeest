if (CMAKE_BINARY_DIR STREQUAL CMAKE_SOURCE_DIR)
    message(FATAL_ERROR "In-tree builds are not supported. Instead, run:\ncmake . -B build <options> && cmake --build build")
endif()

cmake_minimum_required(VERSION 3.20...3.29)

set(CMAKE_CXX_STANDARD 17)
# set(CMAKE_CXX_STANDARD_REQUIRED ON)
# set(CMAKE_CXX_EXTENSIONS OFF)

project(yosys-syn CXX)
set(CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake" ${CMAKE_MODULE_PATH})

if (APPLE)
    # On Linux, the shared libraries can refer to symbols from the executable by default.
    # On macOS, this is not the default behavior and needs to be enabled explicitly.
    set(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} -undefined dynamic_lookup")
endif()

set(OUTPUT_NAME "yosys-syn")

find_package(Yosys)

set(YOSYS_SHARE_DIR ${YOSYS_DATDIR}/share/${OUTPUT_NAME})

add_subdirectory(SRC)
add_subdirectory(ARCHITECTURE)

# Add back once testing is possible
# enable_testing()
# add_subdirectory(tests)
