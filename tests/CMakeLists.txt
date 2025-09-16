
set(ALL_TESTS
    unit/check_commands.ys
    unit/heartbeat-z1000.ys
    unit/heartbeat-z1010.ys
)

foreach(test_script ${ALL_TESTS})
    set(test_fullpath ${CMAKE_CURRENT_SOURCE_DIR}/${test_script})
    get_filename_component(test_dir ${test_fullpath} DIRECTORY)
    add_test(
        NAME ${test_script}
        WORKING_DIRECTORY ${test_dir}
        COMMAND
            ${YOSYS_BINDIR}/yosys -m $<TARGET_FILE:wildebeest> ${test_fullpath}
    )
endforeach()
