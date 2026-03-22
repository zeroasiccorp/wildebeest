
set(ALL_TESTS
    unit/check_commands.ys
    unit/heartbeat-z1000-config.ys
    unit/heartbeat-z1010-config.ys
    unit/heartbeat-z1000-hardcode.ys
    unit/heartbeat-z1010-hardcode.ys
    unit/heartbeat-z1000-config-abspath.ys
    unit/heartbeat-z1010-config-abspath.ys
    unit/dsp/mae-techmap-efpga_mult.ys
    unit/dsp/mae-techmap-efpga_mult_regi.ys
    unit/dsp/mae-techmap-efpga_mult_rego.ys
    unit/dsp/mae-techmap-efpga_mult_regio.ys
    unit/dsp/mae-techmap-efpga_macc.ys
    unit/dsp/mae-techmap-efpga_macc_regi.ys
    # TODO: efpga_macc_pipe and efpga_macc_pipe_regi currently fail. zeroasic_dsp.pmg
    # does not update sigP after absorbing the pipeline register via multHasReg,
    # so the subsequent post-adder search connects to the wrong node and the
    # accumulate feedback is not detected. Both modes map to efpga_mult_rego/regio instead.
    unit/dsp/mae-techmap-efpga_macc_pipe.ys
    unit/dsp/mae-techmap-efpga_macc_pipe_regi.ys
    unit/dsp/mae-techmap-efpga_mult_addc.ys
    unit/dsp/mae-techmap-efpga_mult_addc_regi.ys
    unit/dsp/mae-techmap-efpga_mult_addc_rego.ys
    unit/dsp/mae-techmap-efpga_mult_addc_regio.ys
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
