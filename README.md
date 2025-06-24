# yosys-syn
Zero Asic synthesis plugin for Yosys : 'yosys-syn'

Introduction :
------------
This repository stores the Zero Asic synthesis plugin that provides both FPGA and Asic synthesis flows.

The plugin is called 'yosys-syn' and it provides two top level commands : 

	1/ 'synth_fpga' : this top level command corresponds to 
            the FPGA synthesis flow.  This flow is partially 
            implemented. 
            Once plugin is loaded under Yosys, you need to type 
            "help synth_fpga" to get all the options.

	2/ 'synth_asic' : this top level command corresponds to 
           the Asic synthesis flow. This flow is not implemented 
           yet.

How to build :
-------------
Since the goal it to plug this plugin into a Yosys executable, you need to have a Yosys repository already installed. Under this yosys repository you need to see a 'yosys-config' necessary to build the plugin.

Right under your 'yosys' repository you need to put/copy this 'yosys-syn' plugin.

Then, to build and install 'yosys-syn' under your Yosys repository you have to type :

           make install

It will : 
	o  build yosys-syn.so
        o  copy it under your 'yosys/share/plugins'
        o  copy Zero Asic Architectures files under 'yosys'/share/plugins'.

How to use built 'yosys-syn' plugin with Yosys :
------------------------------------------
When the build is done, you can use the plugin either way: 

            1/ at command line : 

                  yosys -m ./yosys-syn.so -s <script file>

            2/ Or directly in the Yosys executable : 

                  plugin -i yosys-syn
               

