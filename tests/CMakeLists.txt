
set(ALL_TESTS
    unit/check_commands.ys
    unit/heartbeat-z1000-config.ys
    unit/heartbeat-z1010-config.ys
    unit/heartbeat-z1000-hardcode.ys
    unit/heartbeat-z1010-hardcode.ys
    unit/heartbeat-z1000-config-abspath.ys
    unit/heartbeat-z1010-config-abspath.ys
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
