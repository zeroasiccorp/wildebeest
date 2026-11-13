# Runs one yosys test script in its own sandbox directory.
#
#   cmake -DYOSYS=<yosys> -DPLUGIN=<wildebeest.so> -DSCRIPT=<abs .ys path>
#         -DREL_DIR=<dir of the script relative to tests/, e.g. unit/dsp>
#         -DSANDBOX=<per-test scratch dir> -DDATA_DIR=<tests/unit/data>
#         -DTOOLS_DIR=<tests/tools> -P run_yosys_test.cmake
#
# Why a sandbox per test:
#  * synth_fpga and some passes write fixed-name files into the current
#    directory (netlist_synth_fpga.verilog, stat.csv, cp.dot, and ABC's
#    tiny_area_lut4.blif scratch file).  Tests sharing a directory race on
#    those files under 'ctest -j' and crash, and they pollute the source tree.
#  * The sandbox mirrors the script's depth below tests/ and links unit/data
#    back to the source tree, so the relative paths used by the scripts
#    ('data/...', '../data/...') resolve exactly as they do in the source tree.
#
# stdin is /dev/null so 'synth_fpga -wait' cannot block, and TOOLS_DIR is put
# first on PATH so GUI helpers such as 'xdot' resolve to harmless stubs.
#
# An optional '<script>.env' file next to the script holds KEY=VALUE lines that
# are exported for that test only.

foreach(var YOSYS PLUGIN SCRIPT REL_DIR SANDBOX DATA_DIR TOOLS_DIR)
    if(NOT DEFINED ${var})
        message(FATAL_ERROR "run_yosys_test.cmake: ${var} is not set")
    endif()
endforeach()

file(REMOVE_RECURSE "${SANDBOX}")
file(MAKE_DIRECTORY "${SANDBOX}/${REL_DIR}")
file(CREATE_LINK "${DATA_DIR}" "${SANDBOX}/unit/data" SYMBOLIC)

set(ENV{PATH} "${TOOLS_DIR}:$ENV{PATH}")

if(EXISTS "${SCRIPT}.env")
    file(STRINGS "${SCRIPT}.env" env_lines REGEX "^[A-Za-z_][A-Za-z0-9_]*=")
    foreach(line IN LISTS env_lines)
        string(FIND "${line}" "=" eq)
        string(SUBSTRING "${line}" 0 ${eq} key)
        math(EXPR val_start "${eq} + 1")
        string(SUBSTRING "${line}" ${val_start} -1 val)
        set(ENV{${key}} "${val}")
    endforeach()
endif()

execute_process(
    COMMAND "${YOSYS}" -m "${PLUGIN}" "${SCRIPT}"
    WORKING_DIRECTORY "${SANDBOX}/${REL_DIR}"
    INPUT_FILE /dev/null
    RESULT_VARIABLE rc
)

if(NOT rc STREQUAL "0")
    message(FATAL_ERROR "yosys exited with '${rc}' for ${SCRIPT}")
endif()
