# Makefile to build the dynamic plugin 'yosys-syn.so'
#
# Either do "make" or "make yosys-syn" and a "yosys-syn.so" share object file
# will be generated and can be linked with a Yosys executable.
#
# with a Yosys executable it can be plugged-in and used these ways : 
#
#            1/ yosys -m ./yosys-syn.so -s <script file>
#
#            2/ In Yosys do : plugin -i yosys-syn
#               
#
yosys-syn: 
	echo "Building dynamic plugin 'yosys-syn.so'"
	yosys-config --build yosys-syn.so SRC/*.cc 

clean: 
	rm -f yosys-syn.*
