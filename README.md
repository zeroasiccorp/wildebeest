# yosys-syn

The `yosys-syn` project is an open source synthesis plugin for [Yosys](https://github.com/YosysHQ/yosys) with support for the [Zero ASIC Platypus FPGAs](https://www.zeroasic.com/platypus). The plugin also includes experimental configuration support that enables synthesis for hardware targets beyond the Platypus FPGAs.

The `yosys-syn` synthesis recipes represents a significant improvement over existing open source synthesis tools and even compares favorably with state of the art commercial proprietary FPGA tools. The tables below illustrate the synthesis results for various architectures for the [picorv32 CPU](https://raw.githubusercontent.com/zeroasiccorp/logikbench/refs/heads/main/logikbench/blocks/picorv32/rtl/picorv32.v). For a complete set of RTL benchmarks, see the [LogikBench project](https://github.com/zeroasiccorp/logikbench).


## LUT4 Architectures

| Architecture | Tool      | Synthesis Command      | LUTs   | Logic Depth |
|--------------|-----------|------------------------|:------:|:-----------:|
| ice40        | yosys     | synth_ice40            |        |             |
| z1010        | yosys-syn | synth_fpga             |        |             |
| z1010        | yosys-syn | synth_fpga -opt delay  |        |             |

## LUT6 Architectures

| Architecture | Tool      | Synthesis Command      | LUTs   | Logic Depth |
|--------------|-----------|------------------------|:------:|:-----------:|
| Vendor-1     | vendor    | (proprietary)          |        |             |
| Vendor-2     | vendor    | (proprietary)          |        |             |
| xc7          | yosys     | synth_xilinx           |        |             |
| z1XXX        | yosys-syn | synth_fpga             |        |             |
| z1XXX        | yosys-syn | synth_fpga -opt delay  |        |             |

## Prerequisites

* Compiler: >=GCC 11 or >=clang 17
* CMake: 3.20 ... 3.29
* Yosys: >=0.47

## Building

```bash
git clone git@github.com:zeroasiccorp/yosys-syn.git
cd yosys-syn
cmake -S . -B build
cmake --build build
sudo cmake --install build
```

> **NOTE:** The `cmake` flow builds the plugin and copies all files to the yosys executable share directory. As an example, if the `yosys` executable is located at /usr/local/bin/yosys, then the `yosys-syn` plugin will be copied to /usr/local/share/yosys/plugins/yosys-syn.so.

## Quickstart

You can load the `yosys-syn` plugin into Yosys on startup using the -m argument:

```bash
yosys -m yosys-syn
```

Alternatively you can load the plugin at runtime via the `plugin` command:


```bash
yosys> plugin -i yosys-syn
```

Once the plugin is loaded, you are ready to run synthesis.

The following "hello world" example runs synthesis on the [picorv32 CPU](https://raw.githubusercontent.com/zeroasiccorp/logikbench/refs/heads/main/logikbench/blocks/picorv32/rtl/picorv32.v). We encourage you to try your own RTL and report back results.

```bash
read_verilog picorv32.v
hierarchy -check -top picorv32
synth_fpga -partname Z1010
```

## Command Options

To get a listing of all `synth_fpga` options, use the yosys built in `help` command.

```
yosys> help synth_fpga

synth_fpga [options]

This command runs Zero Asic FPGA synthesis flow.

    -top <module>
        Use the specified module as top module, in case more than one exists.

    -opt <mode>
        Specifies optimization mode [area, delay] (default=area).

    -partname <name>
        Specifies architecture partname [Z1000, Z1010]. (default=Z1000).

    -config <file name>
        Specifies config file containing FPGA architecture target parameters.
        If not specified, the -partname value is used to set synthesis target.

    -no_flatten
        Disable design flattening.

    -no_bram
        Disable BRAM inference.

    -no_dsp
        Bypass DSP inference. It is off by default.

    -no_dff_in_dsp
        Specifies to not pack DFF in DSPs. This is off by default.

    -no_xor_tree_process
        Disable xor trees depth reduction in DELAY optimization mode.

    -no_dff_enable
        Disables mapping to DFF with enable.

    -no_dff_async_set
        Disable mapping to DFF with asynchronous set.

    -no_dff_async_reset
        Disable mapping to DFF with asynchronous reset.

    -no_opt_sat_dff
        Disable SAT-based DFF optimizations.

    -no_opt_const_dff
        Disable constant driven DFF optimization.

    -no_sdff
        Disable synchronous set/reset DFF mapping.

    -fsm_encoding [one-hot, binary]
        Specifies FSM encoding : by default a 'one-hot' encoding is performed.

    -resynthesis
        Runs in resynthesis mode which means a lighter touch flow.
        It can be used only after performing a first 'synth_fpga' synthesis pass

    -autoname
        Generate wire and cells names that mimic RTL (instead of generic
        $abc names). Possibly significant runtime impact. (default=off)

    -set_dff_init_value_to_zero
        Set un-initialized DFF to initial value 0. Insert double inverters for DFF
        with initial value 1 and switch its initial value to 0 and modify its
        clear/set/reset functionalities if any.

    -show_dff_init_value
        Show DFF initial values in RTL.

    -continue_if_latch
        Keep running even if latches are inferred.

    -stop_if_undriven_nets
        Stop synthesis if the final netlist has undriven nets.

    -obs_clean
        Overrides built-in Yosys 'opt_clean' function.

    -lut_size
        Specifies lut size. (default=4).

    -verilog <file>
        Writes out design to the specified file.

    -show_max_level
        Show longest paths.

    -csv <file>
        Dump synthesis statistics to file.

    -wait
        Wait after each 'stat' for user to press <enter> key.

    -show_config
        Show the parameters set by the config file.

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
