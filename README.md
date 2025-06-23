# yosys-syn
Zero Asic synthesis plugin for Yosys

Introduction:
------------
This repository stores the Zero Asic synthesis plugin that provides both FPGA and Asic synthesis flows.

The plugin is called 'yosys-syn' and it provides two top level commands : 

	1/ 'synth_fpga' : this top level command corresponds to 
            the FPGA synthesis flow.  This flow is partially implemented. 
            Once plugin is loaded under Yosys, you need to type 
            "help synth_fpga" to get all the options.

	2/ 'synth_asic' : this top level command corresponds to the Asic 
           synthesis flow. This flow is not implemented yet.

How to build:
-------------
You need to have 'yosys-config' executable installed.

Then, to build 'yosys-syn', please type either 'make yosys-syn' or 'make' for short.

A 'yosys-syn.so' share object library file will be generated and can be plugged-in with a Yosys
executable.

To plug-in 'yosys-syn' with a Yosys executable one can do: 

            1/ at command line : 'yosys -m ./yosys-syn.so -s <script file>'

            2/ Or in Yosys exe : plugin -i yosys-syn
               

