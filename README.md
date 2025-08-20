# yosys-syn
Zero Asic synthesis plugin for Yosys.

Introduction :
------------
This repository stores the Zero Asic synthesis plugin that provides both FPGA and Asic synthesis flows.

The plugin is called 'yosys-syn' and it provides two top level commands : 

	1. 'synth_fpga' : this top level command corresponds to 
            the FPGA synthesis flow.  This flow is partially 
            implemented. 
            Once plugin is loaded under Yosys, you need to type 
            "help synth_fpga" to get all the options.

	2. 'synth_asic' : this top level command corresponds to 
           the Asic synthesis flow. This flow is not implemented 
           yet.

How to build the 'yosys-syn' dynamic plugin:
--------------------------------------------

*Prerequisities:*

 * Yosys installed: supported versions are **0.47 0.48 0.49 0.50 0.51 0.52 0.53 0.54 0.55 0.56**

 * C++ compiler: GCC 11 and clang 17 are minimum supported versions

 * Usual toolchains, CMake

To build and install 'yosys-syn':

        cmake -S . -B build
        cmake --build build
        cmake --install build


It will : 

        - build 'yosys-syn.so'

        - copy it under your 'yosys/share/plugins'

        - copy Architectures and support files under 'yosys/share/plugins'.


How to build the 'yosys-syn' dynamic plugin against a different yosys install:
------------------------------------------------------------------------------

To build and install 'yosys-syn':

        cmake -S . -B build -D YOSYS_CONFIG=<path to yosys config>
        cmake --build build
        cmake --install build


How to build the 'yosys-syn' dynamic plugin against an installed version of yosys:
------------------------------------------------------------------------------

To build and install 'yosys-syn':

        cmake -S . -B build -D YOSYS_TREE=<path to yosys git tree>
        cmake --build build
        cmake --install build



How to use 'yosys-syn' plugin with Yosys :
------------------------------------------
When the 'yosys-syn' plugin is built and installed, you can use the plugin either way: 

            1. at command line : 

                  yosys -m yosys-syn -s <script file>

            2. Or directly in the Yosys executable : 

                  plugin -i yosys-syn
               

Example : 
--------

Here is below a classical sequence of commands to install the 'yosys-syn' plugin with Yosys-HQ : 

        // Clone YosysHQ and compile it
        //
        1. git clone https://github.com/YosysHQ/yosys
        2. cd yosys
        3. git submodule update --init --recursive
        4. make

        // Clone 'yosys-syn' under 'yosys' ('yosys' is the directory 
        // where you currently are when you 'cd' in line 2.)
        //
        5. git clone https://github.com/zeroasiccorp/yosys-syn.git
        6. cd yosys-syn
        
        // Install 'yosys-plugin' under 'yosys'
        //
        7. cmake -S . -B build -D YOSYS_TREE=<path to yosys git tree in 2.>
        8. cmake --build build
        9. cmake --install build 

Your 'yosys-syn' plugin can now be part of your 'yosys' executable as soon as you download it.

To download it you can do :
  
        1. yosys -m yosys-syn -s <script file> 

        2. run yosys executable and as first command you can do :
           plugin -i yosys-syn

   
FPGA Configuration file :
------------------------
The configuration file helps to configure important technology parameters used by the
'synth_fpga' command. It is provided through the option 'synth_fpga -config <configuration file>".

The template of this file is as follows:

```
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

Note that all sections like "version", "part_name", "lut_size", ... are required except "root_path", "brams" and "dsps" which are optional.
If "root_path" is not specified, it will correspond to the path where the config file is located.

```
Example : z1010 architecture (with brams and dsps)
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

Example : Using 3rd party BRAM and DSP inference : 
{
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

Example : z1000 architecture (without brams and dsps)
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
