# yosys-syn
Zero Asic synthesis plugin for Yosys : 'yosys-syn'

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

How to build :
-------------
Since the goal is to link this plugin into a Yosys executable, you need to have a Yosys repository already installed. Under this yosys repository you need to see a 'yosys-config' necessary to build the plugin.

Right under your 'yosys' repository you need to put/copy this 'yosys-syn' directory.

Then, to build and install 'yosys-syn' under your Yosys repository, you have to type :

          make install

It will : 

        - build 'yosys-syn.so'

        - copy it under your 'yosys/share/plugins'

        - copy Zero Asic Src/Architectures files under 'yosys'/share/plugins'.

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

        // Clone 'yosys-syn' under 'yosys' ('yosys' is the directory where you currently are)
        //
        5. git clone https://github.com/zeroasiccorp/yosys-syn.git
        6. cd yosys-syn
        
        // Install 'yosys-plugin' under 'yosys'
        //
        7. make install

Your 'yosys-syn' plugin can now be part of your 'yosys' executable as soon as you download it.

To download it you can do :
  
        1. yosys -m yosys-syn -s <script file> 

        2. run yosys executable and as first command you can do :
           plugin -i yosys-syn

   
