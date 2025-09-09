# yosys-syn

The `yosys-syn` project is an open source synthesis plug-in for [Yosys](https://github.com/YosysHQ/yosys) with support for the [Zero ASIC Platypus FPGAs](https://www.zeroasic.com/platypus). The plug-in also includes experimental support for a an FPGA specification format that lowers the barrier to synthesis exploration for a variety of hardware targets.

The `yosys-syn` synthesis recipes represents a significant improvement over existing open source synthesis solutions and compares favorably with commercial proprietary FPGA synthesis tools as shown by the results table below.

!!!RESULTS TABLE GO HERE!!!

## Prerequisites

* Compiler: >=GCC 11 or >=clang 17
* CMake: 3.20 ... 3.29
* Yosys: >=0.47

## Building

The build process relies on a correctly installed yosys distribution. For ubuntu

```bash
git clone git@github.com:zeroasiccorp/yosys-syn.git
cd yosys-syn
cmake -S . -B build
cmake --build build
cmake --install build
```

> **NOTE:** The `cmake` flow builds the plugin and copies all files to the yosys share directory, located in parallel to the `bin` directory.
As an example, if the yosys executable is located at /user/local/bin/yosys, then the `yosys-syn` plugin will be copied to  /usr/local/share/yosys/plugins/yosys-syn.so.

## Quickstart

You can load the `yosys-syn` plugin into Yosys on startup by passing the -m argument:

```bash
yosys -m yosys-syn
```

Alternatively you can load the plugin at runtime via the yosys `plugin` command.


```bash
yosys> plugin -i yosys-syn
```

Once the plugin is loaded, you are ready to run synthesis.

Download the `picorv32` core into your working directory. Alternatively, bring your own code.

The following example illustrates a simple synthesis script for the single file CPU design [picorv32](https://raw.githubusercontent.com/YosysHQ/picorv32/refs/heads/main/picorv32.v).

```bash
read_verilog picorv32.v
synth_fpga -partname Z1010 -opt area
stat
```



## Command Options

To get a listing of all `synth_fpga` options, use the yosys built in `help` command.

```
yosys> help synth_fpga

synth_fpga [options]

This command runs Zero Asic FPGA synthesis flow.

    -top <module>
        use the specified module as top module

    -config <file name>
        Specifies the config file setting main 'synth_fpga' parameters.

    -show_config
        Show the parameters set by the config file.

    -no_flatten
        skip flatening. By default, design is flatened.

    -opt
        specifies the optimization target : area, delay, default, fast.

    -partname
        Specifies the Architecture partname used. By default it is Z1000.

    -no_bram
        Bypass BRAM inference. It is off by default.

    -use_bram_tech [zeroasic, microchip]
        Invoke architecture specific DSP inference. It is off by default. -no_bram
        overides -use_BRAM_TECH.

    -no_dsp
        Bypass DSP inference. It is off by default.

    -use_dsp_tech [zeroasic, bare_mult, mae]
        Invoke architecture specific DSP inference. It is off by default. -no_dsp
        overides -use_dsp_tech.

    -do_not_pack_dff_in_dsp
        Specifies to not pack DFF in DSPs. This is off by default.

    -fsm_encoding [one-hot, binary]
        Specifies FSM encoding : by default a 'one-hot' encoding is performed.

    -resynthesis
        switch synthesis flow to resynthesis mode which means a lighter flow.
        It can be used only after performing a first 'synth_fpga' synthesis pass

    -insbuf
        performs buffers insertion (Off by default).

    -no_xor_tree_process
        Disable xor trees depth reduction for DELAY mode (Off by default).

    -autoname
        Generate, if possible, better wire and cells names close to RTL names rather than
        $abc generic names. This is off by default. Be careful because it may blow up runtime.

    -no_dff_enable
        specifies that DFF with enable feature is not supported. By default,
        DFF with enable is supported.

    -no_dff_async_set
        specifies that DFF with asynchronous set feature is not supported. By default,
        DFF with asynchronous set is supported.

    -no_dff_async_reset
        specifies that DFF with asynchronous reset feature is not supported. By default,
        DFF with asynchronous reset is supported.

    -no_opt_sat_dff
        Disable SAT-based DFF optimizations. This is off by default.

    -no_opt_const_dff
        Disable constant driven DFF optimization as it can create simulation differences (since it may ignore DFF init values in some cases). This is off by default.

    -set_dff_init_value_to_zero
        Set un-initialized DFF to initial value 0. Insert double inverters for DFF with initial value 1 and switch its initial value to 0 and modify its clear/set/reset functionalities if any. This is off by default.

    -show_dff_init_value
        Show all DFF initial values coming from the original RTL. This is off by default.

    -continue_if_latch
        Keep running Synthesis even if some Latch inference is involved. The final netlist will not be valid but it can be usefull to get the final netlist stats. This is off by default.

    -no_sdff
        Disable synchronous set/reset DFF mapping. It is off by default.

    -stop_if_undriven_nets
        Stop Synthesis if the final netlist has undriven nets. This is off by default.

    -obs_clean
        specifies to use 'obs_clean' cleanup function instead of regular
        'opt_clean'. This is off by default.

    -lut_size
        specifies lut size. By default lut size is 4.

    -verilog <file>
        write the design to the specified Verilog netlist file. writing of an
        output file is omitted if this parameter is not specified.

    -show_max_level
        Show longest paths. This is off by default except if we are in delay mode.

    -csv
        Dump a 'stat.csv' file. This is off by default.

    -wait
        wait after each 'stat' report for user to touch <enter> key. Help for
        flow analysis/debug.

```

## FPGA Specification Format
------------------------
The FPGA configuration option makes the `synth_fpga` command a powerful general purpose synthesis plugin rather than a bespoke command targeted at a single FPGA hardware target.

```
yosys > synth_fpga -config <configuration file>
```

The configuration format is still work in progress. An draft format specification is shown below.

```json
{
        "version": <int, version of file schema, current version is 1>,
        "partname": <str, name of the fpga part>,
        "root_path": <path, absolute path where all other paths are relative to>, (optional)
        "lut_size": <int, size of lut>,
        "flipflops": {
                "features": [<str, list of features file async_set, flop_enable, async_reset, etc.>]
                "models": {
                        "<str, ff name>": <path, relative path to DFFs model file>
                },
                "techmap": <path, relative path to yosys DFFs tech mapping file>
        },
        "brams": { (optional)
                "memory_libmap": [<path, relative path to yosys memory mapping file>, ...]
                "memory_libmap_parameters": [<parameter setting>, ...] (optional)
                "techmap": [<path, relative path to yosys memory tech mapping>, ...]
        },
        "dsps": { (optional)
                "family": <str, name of dsp family>,
                "techmap": <path, relative path to yosys dsp tech mapping file>,
                "techmap_parameters": {
                        "<str, name of define>": <str or int, value of define>
                }
                "pack_command": "DSP packing command" (optional)
        }
}
```

Sections version, partname, lut_size, flip-flops, are required. Sections root_path, brams,and dsps are optional. If "root_path" is not specified, it will correspond to the path where the config file is located.

### Logic Only Example: No BRAMs or DSPs

```
{
    "version": 1,
    "partname": "z1000",
    "lut_size": 4,
    "flipflops": {
        "features": ["async_reset", "async_set", "flop_enable"],
        "models": {
            "dffers": "/ffs/dffers.v",
            "dffer": "/ffs/dffer.v",
            "dffes": "/ffs/dffes.v",
            "dffe": "/ffs/dffe.v",
            "dffrs": "/ffs/dffrs.v",
            "dffr": "/ffs/dffr.v",
            "dffs": "/ffs/dffs.v",
            "dff": "/ffs/dff.v"
        },
        "techmap": "/tech_flops.v"
    }
}
```
### Full Example: With BRAMs and DSPs

**Example One:

```json
{
    "version": 1,
    "partname": "z1010",
    "lut_size": 4,
    "flipflops": {
        "features": ["async_reset", "async_set", "flop_enable"],
        "models": {
            "dffers": "/ffs/dffers.v",
            "dffer": "/ffs/dffer.v",
            "dffes": "/ffs/dffes.v",
            "dffe": "/ffs/dffe.v",
            "dffrs": "/ffs/dffrs.v",
            "dffr": "/ffs/dffr.v",
            "dffs": "/ffs/dffs.v",
            "dff": "/ffs/dff.v"
        },
        "techmap": "/tech_flops.v"
    },
    "brams": {
        "memory_libmap": ["/bram_memory_map.txt"]
        "techmap": ["/tech_bram.v"]
    },
    "dsps": {
        "family": "DSP48",
        "techmap": "/mult18x18_DSP48.v",
        "techmap_parameters": {
            "DSP_A_MAXWIDTH": 18,
            "DSP_B_MAXWIDTH": 18,
            "DSP_A_MINWIDTH": 2,
            "DSP_B_MINWIDTH": 2,
            "DSP_Y_MINWIDTH": 9,
            "DSP_SIGNEDONLY": 1,
            "DSP_NAME": "$__MUL18X18"
        }
    }
}
```
### Third Party Example: With BRAM and DSPs
```json
{
  "version": 2,
  "partname": "Z1010",
  "lut_size": 4,
  "root_path" : "/home/user/YOSYS_DYN/yosys/yosys-syn/",
  "flipflops": {
                "features": ["async_reset", "async_set", "flop_enable"]
                "models": {
                        "dffers": "SRC/ff_models/dffers.v",
                        "dffer": "SRC/ff_models/dffer.v",
                        "dffes": "SRC/ff_models/dffes.v",
                        "dffe": "SRC/ff_models/dffe.v",
                        "dffrs": "SRC/ff_models/dffrs.v",
                        "dffr": "SRC/ff_models/dffr.v",
                        "dffs": "SRC/ff_models/dffs.v",
                        "dff": "SRC/ff_models/dff.v"
                },
                "techmap": "architecture/Z1010/techlib/tech_flops.v"
        },
  "brams": {
            "memory_libmap": [ "architecture/Z1010/bram/LSRAM.txt",
                               "architecture/Z1010/bram/uSRAM.txt"]
            "memory_libmap_parameters": ["-logic-cost-rom 0.5"]
            "techmap": ["architecture/Z1010/bram/LSRAM_map.v",
                        "architecture/Z1010/bram/uSRAM_map.v"]
        },
  "dsps": {
          "family": "microchip",
          "techmap": "../techlibs/microchip/polarfire_dsp_map.v",
          "techmap_parameters": {
                   "DSP_A_MAXWIDTH": 18,
                   "DSP_B_MAXWIDTH": 18,
                   "DSP_A_MAXWIDTH_PARTIAL": 18,
                   "DSP_A_MINWIDTH": 2,
                   "DSP_B_MINWIDTH": 2,
                   "DSP_Y_MINWIDTH": 9,
                   "DSP_SIGNEDONLY": 1,
                   "DSP_NAME": "$__MUL18X18"
          },
          "pack_command": "microchip_dsp -family polarfire"
       }
}
```
