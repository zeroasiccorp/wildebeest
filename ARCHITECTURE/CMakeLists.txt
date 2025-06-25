include(PMGen)

pmgen_lib(
    NAME z1010_dsp_cascade
    PMG_FILE Z1010/DSP/dsp_cascade.pmg
)

pmgen_lib(
    NAME z1010_dsp_CREG
    PMG_FILE Z1010/DSP/dsp_CREG.pmg
)
set(Z1010_VERILOG_FILES
    Z1010/DSP/DSP48_sim.v
    Z1010/DSP/mult18x18_DSP48.v)
install(FILES ${Z1010_VERILOG_FILES} DESTINATION ${YOSYS_DATDIR}/share/yosys-syn/Z1010/DSP)
