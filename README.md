# yosys-syn

The `yosys-syn` project is a dynamically configurable synthesis plugin for [Yosys](https://github.com/YosysHQ/yosys) with support for the [Zero ASIC Platypus FPGAs](https://www.zeroasic.com/platypus). 

The plugin also supports other popular FPGAs via a standardized JSON FPGA specification format.

## Prerequisites

* Compiler: >=GCC 11 or >=clang 17
* CMake: 3.20 .. 3.29 

## Building

First you will need to clone and build `yosys` from source. See the [Yosys readme](https://github.com/YosysHQ/yosys) for complete build instructions. 

```bash
# build yosys
git clone --recurse-submodules https://github.com/YosysHQ/yosys.git
cd yosys
make -j$(nproc)
sudo make install
# build yosys-sy nplugin
git clone git@github.com:zeroasiccorp/yosys-syn.git
cd yosys-syn
cmake -S . -B build -D YOSYS_TREE=<path to Yosys source-dir>
cmake --build build
cmake --install build
```

The build process:
1. Builds the plugin `yosys-syn.so`
2. Copies the yosys-syn.so file to "\<path\>/yosys/share/plugins"
3. Copies architectures files to "\<path\>/yosys/share/plugins"

> **NOTE:** We are working on optimizing the build process to remove the need for recompiling yosys. 


## Quickstart

You can load the `yosys-syn` plugin into Yosys on startup by passing the -m argument:

```bash
yosys -m yosys-syn
```

Alternatively, you can load the plugin at runtime via the yosys `plugin` command.


```bash
yosys> plugin -i yosys-syn
```

Once the plugin is loaded, you are ready to run synthesis.

Download the `picorv32` core into your working directory. Alternatively, bring your own code.

The following example illustrates a simple synthesis script for the single file CPU design [picorv32](https://raw.githubusercontent.com/YosysHQ/picorv32/refs/heads/main/picorv32.v).

```bash
read_verilog picorv32.v
synth_fpga -partname Z1010
stat
```

A listing of all of the `synth_fpga` options via the yosys built in `help` command.

```
yosys> help synth_fpga
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

Sections version, partname, lut_size, flip-flops, a required. Sections root_path, brams,and dsps are optional. If "root_path" is not specified, it will correspond to the path where the config file is located.

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
                        "dffers": "SRC/FF_MODELS/dffers.v",
                        "dffer": "SRC/FF_MODELS/dffer.v",
                        "dffes": "SRC/FF_MODELS/dffes.v",
                        "dffe": "SRC/FF_MODELS/dffe.v",
                        "dffrs": "SRC/FF_MODELS/dffrs.v",
                        "dffr": "SRC/FF_MODELS/dffr.v",
                        "dffs": "SRC/FF_MODELS/dffs.v",
                        "dff": "SRC/FF_MODELS/dff.v"
                },
                "techmap": "ARCHITECTURE/Z1010/techlib/tech_flops.v"
        },
  "brams": {
            "memory_libmap": [ "ARCHITECTURE/Z1010/BRAM/LSRAM.txt",
                               "ARCHITECTURE/Z1010/BRAM/uSRAM.txt"]
            "memory_libmap_parameters": ["-logic-cost-rom 0.5"]
            "techmap": ["ARCHITECTURE/Z1010/BRAM/LSRAM_map.v",
                        "ARCHITECTURE/Z1010/BRAM/uSRAM_map.v"]
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
