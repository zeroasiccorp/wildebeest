# C++ unit tests: wildebeest_cxxtest.so is the wildebeest plugin in which some
# source files are replaced by test files that '#include' them (so the tests
# can reach code in the sources' anonymous namespaces), plus the 'wb_cxxtest'
# pass that runs the cases.  It is loaded instead of wildebeest.so, since both
# register the same pass names.
#
# Each '<name>_test.cc' that includes '../../src/<name>.cc' takes that source
# out of the regular list; the remaining sources are compiled as usual.

file(GLOB CXX_TEST_SOURCES CONFIGURE_DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/*_test.cc)
file(GLOB PLUGIN_SOURCES CONFIGURE_DEPENDS ${CMAKE_SOURCE_DIR}/src/*.cc)

set(INCLUDED_SOURCES)
foreach(test_src ${CXX_TEST_SOURCES})
    file(STRINGS ${test_src} includes REGEX "^#include \"\\.\\./\\.\\./src/[a-z_]+\\.cc\"")
    foreach(inc ${includes})
        string(REGEX REPLACE "^#include \"\\.\\./\\.\\./src/([a-z_]+\\.cc)\"" "\\1" src "${inc}")
        list(APPEND INCLUDED_SOURCES ${CMAKE_SOURCE_DIR}/src/${src})
    endforeach()
endforeach()
list(REMOVE_ITEM PLUGIN_SOURCES ${INCLUDED_SOURCES})

add_library(wildebeest_cxxtest SHARED
    ${CXX_TEST_SOURCES}
    ${PLUGIN_SOURCES}
    ${CMAKE_CURRENT_SOURCE_DIR}/wb_test_main.cc
)
target_include_directories(wildebeest_cxxtest
    PRIVATE
        ${CMAKE_BINARY_DIR}/src
        ${CMAKE_SOURCE_DIR}/src
        ${CMAKE_CURRENT_SOURCE_DIR}
)
target_link_libraries(wildebeest_cxxtest
    PRIVATE
        yosys::yosys
        zeroasic_dsp
)
target_compile_definitions(wildebeest_cxxtest
    PRIVATE
        SYN_SHARE_DIR=\"plugins/${OUTPUT_NAME}\"
)
add_dependencies(wildebeest_cxxtest wildebeest)  # for the generated version.h
set_target_properties(wildebeest_cxxtest PROPERTIES PREFIX "" SUFFIX ".so")
if (WILDEBEEST_COVERAGE)
    target_compile_options(wildebeest_cxxtest PRIVATE --coverage -O0 -g -fprofile-update=atomic)
    target_link_options(wildebeest_cxxtest PRIVATE --coverage)
endif()

# One CTest entry per WB_TEST case, each in its own Yosys process and its own
# working directory (cases write files such as wbtest_input.v).
foreach(test_src ${CXX_TEST_SOURCES})
    get_filename_component(test_file ${test_src} NAME_WE)
    file(STRINGS ${test_src} cases REGEX "^WB_TEST\\([A-Za-z0-9_]+\\)")
    foreach(case_line ${cases})
        string(REGEX REPLACE "^WB_TEST\\(([A-Za-z0-9_]+)\\).*" "\\1" case_name "${case_line}")
        set(case_dir ${CMAKE_BINARY_DIR}/tests/cxx/${case_name})
        file(MAKE_DIRECTORY ${case_dir})
        add_test(
            NAME cxx/${test_file}/${case_name}
            WORKING_DIRECTORY ${case_dir}
            COMMAND ${YOSYS_BINDIR}/yosys -m $<TARGET_FILE:wildebeest_cxxtest>
                    -p "wb_cxxtest -case ${case_name}"
        )
        # The stub 'xdot' in tests/tools keeps '-dot' runs from opening a GUI.
        set_tests_properties(cxx/${test_file}/${case_name} PROPERTIES
            LABELS "cxx"
            ENVIRONMENT "PATH=${CMAKE_SOURCE_DIR}/tests/tools:$ENV{PATH}")
    endforeach()
endforeach()
