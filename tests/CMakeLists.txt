
# Every *.ys script below unit/ is a test.  Scripts are discovered rather than
# listed so a new test cannot be silently left out.  unit/data/ only holds
# fixtures (no .ys files).
file(GLOB_RECURSE ALL_TESTS
    RELATIVE ${CMAKE_CURRENT_SOURCE_DIR}
    CONFIGURE_DEPENDS
    ${CMAKE_CURRENT_SOURCE_DIR}/unit/*.ys
)
list(SORT ALL_TESTS)

# Scripts that must not be registered (keep empty unless there is a reason,
# and give the reason next to the entry).
set(EXCLUDED_TESTS
)
list(REMOVE_ITEM ALL_TESTS ${EXCLUDED_TESTS})

set(TEST_SANDBOX_ROOT ${CMAKE_BINARY_DIR}/tests/sandbox)

foreach(test_script ${ALL_TESTS})
    set(test_fullpath ${CMAKE_CURRENT_SOURCE_DIR}/${test_script})
    get_filename_component(test_rel_dir ${test_script} DIRECTORY)
    get_filename_component(test_name_we ${test_script} NAME_WE)

    # Labels come from the directory path below unit/, e.g.
    # unit/synth_fpga/dsp/x.ys -> "synth_fpga;dsp".  Top-level scripts get
    # "smoke".  Scripts named *-slow.ys also get "slow" and are skipped by the
    # regular build matrix (ctest -LE slow).
    string(REGEX REPLACE "^unit/?" "" label_path "${test_rel_dir}")
    if(label_path STREQUAL "")
        set(test_labels smoke)
    else()
        string(REPLACE "/" ";" test_labels "${label_path}")
    endif()
    if(test_name_we MATCHES "-slow$")
        list(APPEND test_labels slow)
    endif()

    string(REGEX REPLACE "[^A-Za-z0-9_.-]" "_" sandbox_name "${test_script}")

    add_test(
        NAME ${test_script}
        COMMAND ${CMAKE_COMMAND}
            -DYOSYS=${YOSYS_BINDIR}/yosys
            -DPLUGIN=$<TARGET_FILE:wildebeest>
            -DSCRIPT=${test_fullpath}
            -DREL_DIR=${test_rel_dir}
            -DSANDBOX=${TEST_SANDBOX_ROOT}/${sandbox_name}
            -DDATA_DIR=${CMAKE_CURRENT_SOURCE_DIR}/unit/data
            -DTOOLS_DIR=${CMAKE_CURRENT_SOURCE_DIR}/tools
            -P ${CMAKE_CURRENT_SOURCE_DIR}/run_yosys_test.cmake
    )
    set_tests_properties(${test_script} PROPERTIES LABELS "${test_labels}")
endforeach()
