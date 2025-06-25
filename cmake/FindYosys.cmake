# https://github.com/povik/yosys-slang/blob/76b83eb5b73ba871797e6db7bc5fed10af380be4/cmake/FindYosys.cmake
set(YOSYS_CONFIG "yosys-config" CACHE STRING "Location of yosys-config utility")
message(STATUS "Using yosys: ${YOSYS_CONFIG}")

execute_process(
    COMMAND ${YOSYS_CONFIG} --bindir
    OUTPUT_VARIABLE YOSYS_BINDIR 
    OUTPUT_STRIP_TRAILING_WHITESPACE
    COMMAND_ERROR_IS_FATAL ANY
)
message(STATUS "yosys-config --bindir: ${YOSYS_BINDIR}")

execute_process(
    COMMAND ${YOSYS_CONFIG} --datdir
    OUTPUT_VARIABLE YOSYS_DATDIR
    OUTPUT_STRIP_TRAILING_WHITESPACE
    COMMAND_ERROR_IS_FATAL ANY
)
message(STATUS "yosys-config --datdir: ${YOSYS_DATDIR}")

execute_process(
    COMMAND ${YOSYS_CONFIG} --cxxflags
    OUTPUT_VARIABLE YOSYS_CXXFLAGS
    OUTPUT_STRIP_TRAILING_WHITESPACE
    COMMAND_ERROR_IS_FATAL ANY
)
string(REGEX REPLACE " +" ";" YOSYS_CXXFLAGS ${YOSYS_CXXFLAGS})
list(FILTER YOSYS_CXXFLAGS INCLUDE REGEX "^-[ID]")
message(STATUS "yosys-config --cxxflags (filtered): ${YOSYS_CXXFLAGS}")

add_library(yosys::yosys INTERFACE IMPORTED)
target_compile_options(yosys::yosys INTERFACE ${YOSYS_CXXFLAGS})

if(WIN32)
    execute_process(
        COMMAND ${YOSYS_CONFIG} --linkflags
        OUTPUT_VARIABLE YOSYS_LINKFLAGS
        OUTPUT_STRIP_TRAILING_WHITESPACE
        COMMAND_ERROR_IS_FATAL ANY
    )
    string(REGEX REPLACE " +" ";" YOSYS_LINKFLAGS ${YOSYS_LINKFLAGS})
    list(FILTER YOSYS_LINKFLAGS INCLUDE REGEX "^-[L]")
    message(STATUS "yosys-config --linkflags (filtered): ${YOSYS_LINKFLAGS}")

    target_link_options(yosys::yosys INTERFACE ${YOSYS_LINKFLAGS})

    target_link_libraries(yosys::yosys INTERFACE yosys_exe)
endif()

set(YOSYS_BINDIR ${YOSYS_BINDIR})
set(YOSYS_DATDIR ${YOSYS_DATDIR})
