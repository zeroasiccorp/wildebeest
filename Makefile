# This yosys-syn plugin is right under the Yosys dicrectory where we 
# want to link this plugin.
# So the YOSYS_PATH is right above
#
YOSYS_PATH ?= $(realpath ..)

# And the yosys-config needed to plug this plugin is the one
# under YOSYS_PATH
#
YOSYS_CONFIG ?= $(YOSYS_PATH)/yosys-config

ifeq (,$(wildcard $(YOSYS_CONFIG)))
$(error "Didn't find 'yosys-config' under '$(YOSYS_PATH)'")
endif

CXX ?= $(shell $(YOSYS_CONFIG) --cxx)
CXXFLAGS ?= $(shell $(YOSYS_CONFIG) --cxxflags) #-DSDC_DEBUG
LDFLAGS ?= $(shell $(YOSYS_CONFIG) --ldflags)
LDLIBS ?= $(shell $(YOSYS_CONFIG) --ldlibs)
PLUGINS_DIR ?= $(shell $(YOSYS_PATH) --datdir)/share/plugins
DATA_DIR ?= $(shell $(YOSYS_CONFIG) --datdir)
EXTRA_FLAGS ?= 

export PATH := $(YOSYS_PATH):$(PATH)

ARCHITECTURE		= ARCHITECTURE
SRC			= SRC
VERILOG_MODULES	= $(ARCHITECTURE)/Z1010/DSP/DSP48_sim.v \
		  $(ARCHITECTURE)/Z1010/DSP/mult18x18_DSP48.v \
		  $(ARCHITECTURE)/Z1010/BRAM/brams_map.v \
		  $(ARCHITECTURE)/Z1010/BRAM/lutrams_map.v \
		  $(ARCHITECTURE)/Z1010/BRAM/tech_bram.v \
		  $(SRC)/FF_MODELS/dffers.v \
		  $(SRC)/FF_MODELS/dffer.v \
		  $(SRC)/FF_MODELS/dffes.v \
		  $(SRC)/FF_MODELS/dffe.v \
		  $(SRC)/FF_MODELS/dffrs.v \
		  $(SRC)/FF_MODELS/dffr.v \
		  $(SRC)/FF_MODELS/dffs.v \
		  $(SRC)/FF_MODELS/dff.v 

NAME = yosys-syn
SOURCES = $(SRC)/clk_domains.cc \
	  $(SRC)/load_models.cc \
  	  $(SRC)/report_stat.cc  \
	  $(SRC)/time_chrono.cc \
	  $(SRC)/dsp.cc \
	  $(SRC)/obs_clean.cc \
	  $(SRC)/zopt_dff.cc \
	  $(SRC)/zqcsat.cc \
  	  $(SRC)/synth_fpga.cc

DEPS = pmgen/dsp_cascade_pm.h \
       pmgen/dsp_CREG_pm.h \
       $(ARCHITECTURE)/Z1010/BRAM/bram_memory_map.txt \
       $(ARCHITECTURE)/Z1010/BRAM/brams.txt \
       $(ARCHITECTURE)/Z1010/BRAM/lutrams.txt 


all: $(NAME).so
	echo "Plugin $(NAME).so has been generated"
	ls -l $(NAME).so

pmgen:
	mkdir -p pmgen

pmgen/dsp_cascade_pm.h: ARCHITECTURE/Z1010/DSP/dsp_cascade.pmg | pmgen
	python3 pmgen.py -o $@ -p dsp_cascade_pm ARCHITECTURE/Z1010/DSP/dsp_cascade.pmg

pmgen/dsp_CREG_pm.h:  ARCHITECTURE/Z1010/DSP/dsp_CREG.pmg | pmgen
	python3 pmgen.py -o $@ -p dsp_CREG_pm ARCHITECTURE/Z1010/DSP/dsp_CREG.pmg

# We have the 'pmgen.py' already here but in case we would need to get it back here is
# how.
# pmgen.py:
# 	wget -nc -O $@ https://raw.githubusercontent.com/YosysHQ/yosys/master/passes/pmgen/pmgen.py

OBJS := $(SOURCES:cc=o)

$(OBJS): %.o: %.cc $(DEPS)
	$(CXX) -I$(YOSYS_PATH) $(CXXFLAGS) $(CPPFLAGS) $(EXTRA_FLAGS) -std=c++17 -c -o $@ $(filter %.cc, $^)

$(NAME).so: $(OBJS)
	$(CXX) $(CXXFLAGS) $(LDFLAGS) -shared -o $@ $^ $(LDLIBS)

install_plugin: $(NAME).so
	mkdir -p $(YOSYS_PATH)/share/plugins
	cp $(NAME).so $(YOSYS_PATH)/share/plugins/$(NAME).so
	echo "$(NAME).so has been installed under $(YOSYS_PATH)/share/plugins/$(NAME).so"
	ls -l $(YOSYS_PATH)/share/plugins/$(NAME).so
	mkdir -p $(YOSYS_PATH)/share/$(NAME)
	cp -R $(ARCHITECTURE) $(YOSYS_PATH)/share/$(NAME)
	cp -R $(SRC) $(YOSYS_PATH)/share/$(NAME)

.PHONY: install
install: install_plugin 

clean:
	rm -rf SRC/*.d SRC/*.o *.so pmgen/ $(YOSYS_PATH)/share/plugins

