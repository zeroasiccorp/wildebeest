#
# Makefile.inc is used to compile 'yosys-syn' with the global Makefile used to create the main Yosys executable.
#
OBJS += techlibs/yosys-syn/SRC/synth_fpga.o techlibs/yosys-syn/SRC/clk_domains.o techlibs/yosys-syn/SRC/load_models.o techlibs/yosys-syn/SRC/report_stat.o techlibs/yosys-syn/SRC/time_chrono.o techlibs/yosys-syn/SRC/obs_clean.o techlibs/yosys-syn/SRC/zopt_dff.o techlibs/yosys-syn/SRC/zqcsat.o

$(eval $(call add_share_file,share/yosys-syn/ARCHITECTURE/Z1000/techlib,techlibs/yosys-syn/ARCHITECTURE/Z1000/techlib/bram_memory_map_empty.txt))
$(eval $(call add_share_file,share/yosys-syn/ARCHITECTURE/Z1000/techlib,techlibs/yosys-syn/ARCHITECTURE/Z1000/techlib/tech_bram_empty.v))
$(eval $(call add_share_file,share/yosys-syn/ARCHITECTURE/Z1000/techlib,techlibs/yosys-syn/ARCHITECTURE/Z1000/techlib/tech_dsp_empty.v))
$(eval $(call add_share_file,share/yosys-syn/ARCHITECTURE/Z1000/techlib,techlibs/yosys-syn/ARCHITECTURE/Z1000/techlib/tech_flops.v))

$(eval $(call add_share_file,share/yosys-syn/ARCHITECTURE/Z1010/techlib,techlibs/yosys-syn/ARCHITECTURE/Z1010/techlib/bram_memory_map_empty.txt))
$(eval $(call add_share_file,share/yosys-syn/ARCHITECTURE/Z1010/techlib,techlibs/yosys-syn/ARCHITECTURE/Z1010/techlib/tech_bram_empty.v))
$(eval $(call add_share_file,share/yosys-syn/ARCHITECTURE/Z1010/techlib,techlibs/yosys-syn/ARCHITECTURE/Z1010/techlib/tech_dsp_empty.v))
$(eval $(call add_share_file,share/yosys-syn/ARCHITECTURE/Z1010/techlib,techlibs/yosys-syn/ARCHITECTURE/Z1010/techlib/tech_flops.v))


#########################
# LUT4
#########################
#

# BEST Flows
#
$(eval $(call add_share_file,share/yosys-syn/SRC/ABC_SCRIPTS/LUT4/BEST,techlibs/yosys-syn/SRC/ABC_SCRIPTS/LUT4/BEST/area_lut4.scr))
$(eval $(call add_share_file,share/yosys-syn/SRC/ABC_SCRIPTS/LUT4/BEST,techlibs/yosys-syn/SRC/ABC_SCRIPTS/LUT4/BEST/delay_lut4.scr))
$(eval $(call add_share_file,share/yosys-syn/SRC/ABC_SCRIPTS/LUT4/BEST,techlibs/yosys-syn/SRC/ABC_SCRIPTS/LUT4/BEST/fast_lut4.scr))

#V1
$(eval $(call add_share_file,share/yosys-syn/SRC/ABC_SCRIPTS/LUT4/V1,techlibs/yosys-syn/SRC/ABC_SCRIPTS/LUT4/V1/area_lut4.scr))
$(eval $(call add_share_file,share/yosys-syn/SRC/ABC_SCRIPTS/LUT4/V1,techlibs/yosys-syn/SRC/ABC_SCRIPTS/LUT4/V1/delay_lut4.scr))
$(eval $(call add_share_file,share/yosys-syn/SRC/ABC_SCRIPTS/LUT4/V1,techlibs/yosys-syn/SRC/ABC_SCRIPTS/LUT4/V1/default_lut4.scr))
$(eval $(call add_share_file,share/yosys-syn/SRC/ABC_SCRIPTS/LUT4/V1,techlibs/yosys-syn/SRC/ABC_SCRIPTS/LUT4/V1/fast_lut4.scr))

#V2
$(eval $(call add_share_file,share/yosys-syn/SRC/ABC_SCRIPTS/LUT4/V2,techlibs/yosys-syn/SRC/ABC_SCRIPTS/LUT4/V2/default_lut4.scr))
$(eval $(call add_share_file,share/yosys-syn/SRC/ABC_SCRIPTS/LUT4/V2,techlibs/yosys-syn/SRC/ABC_SCRIPTS/LUT4/V2/delay_lut4.scr))
$(eval $(call add_share_file,share/yosys-syn/SRC/ABC_SCRIPTS/LUT4/V2,techlibs/yosys-syn/SRC/ABC_SCRIPTS/LUT4/V2/area_lut4.scr))
$(eval $(call add_share_file,share/yosys-syn/SRC/ABC_SCRIPTS/LUT4/V2,techlibs/yosys-syn/SRC/ABC_SCRIPTS/LUT4/V2/fast_lut4.scr))

#V3
$(eval $(call add_share_file,share/yosys-syn/SRC/ABC_SCRIPTS/LUT4/V3,techlibs/yosys-syn/SRC/ABC_SCRIPTS/LUT4/V3/area_lut4.scr))
$(eval $(call add_share_file,share/yosys-syn/SRC/ABC_SCRIPTS/LUT4/V3,techlibs/yosys-syn/SRC/ABC_SCRIPTS/LUT4/V3/delay_lut4.scr))
$(eval $(call add_share_file,share/yosys-syn/SRC/ABC_SCRIPTS/LUT4/V3,techlibs/yosys-syn/SRC/ABC_SCRIPTS/LUT4/V3/fast_lut4.scr))

#V4
$(eval $(call add_share_file,share/yosys-syn/SRC/ABC_SCRIPTS/LUT4/V4,techlibs/yosys-syn/SRC/ABC_SCRIPTS/LUT4/V4/area_lut4.scr))
$(eval $(call add_share_file,share/yosys-syn/SRC/ABC_SCRIPTS/LUT4/V4,techlibs/yosys-syn/SRC/ABC_SCRIPTS/LUT4/V4/delay_lut4.scr))
$(eval $(call add_share_file,share/yosys-syn/SRC/ABC_SCRIPTS/LUT4/V4,techlibs/yosys-syn/SRC/ABC_SCRIPTS/LUT4/V4/fast_lut4.scr))


#########################
# LUT6
#########################
#
# BEST Flows
#
$(eval $(call add_share_file,share/yosys-syn/SRC/ABC_SCRIPTS/LUT6/BEST,techlibs/yosys-syn/SRC/ABC_SCRIPTS/LUT6/BEST/area_lut6.scr))
$(eval $(call add_share_file,share/yosys-syn/SRC/ABC_SCRIPTS/LUT6/BEST,techlibs/yosys-syn/SRC/ABC_SCRIPTS/LUT6/BEST/delay_lut6.scr))
$(eval $(call add_share_file,share/yosys-syn/SRC/ABC_SCRIPTS/LUT6/BEST,techlibs/yosys-syn/SRC/ABC_SCRIPTS/LUT6/BEST/fast_lut6.scr))

$(eval $(call add_share_file,share/yosys-syn/SRC/ABC_SCRIPTS/LUT6/V1,techlibs/yosys-syn/SRC/ABC_SCRIPTS/LUT6/V1/area_lut6.scr))
$(eval $(call add_share_file,share/yosys-syn/SRC/ABC_SCRIPTS/LUT6/V1,techlibs/yosys-syn/SRC/ABC_SCRIPTS/LUT6/V1/delay_lut6.scr))
$(eval $(call add_share_file,share/yosys-syn/SRC/ABC_SCRIPTS/LUT6/V1,techlibs/yosys-syn/SRC/ABC_SCRIPTS/LUT6/V1/default_lut6.scr))

# MODELS
#
$(eval $(call add_share_file,share/yosys-syn/SRC/FF_MODELS,techlibs/yosys-syn/SRC/FF_MODELS/dff.v))

$(eval $(call add_share_file,share/yosys-syn/SRC/FF_MODELS,techlibs/yosys-syn/SRC/FF_MODELS/dffe.v))

$(eval $(call add_share_file,share/yosys-syn/SRC/FF_MODELS,techlibs/yosys-syn/SRC/FF_MODELS/dffr.v))

$(eval $(call add_share_file,share/yosys-syn/SRC/FF_MODELS,techlibs/yosys-syn/SRC/FF_MODELS/dffs.v))

$(eval $(call add_share_file,share/yosys-syn/SRC/FF_MODELS,techlibs/yosys-syn/SRC/FF_MODELS/dffrs.v))

$(eval $(call add_share_file,share/yosys-syn/SRC/FF_MODELS,techlibs/yosys-syn/SRC/FF_MODELS/dffer.v))

$(eval $(call add_share_file,share/yosys-syn/SRC/FF_MODELS,techlibs/yosys-syn/SRC/FF_MODELS/dffes.v))

$(eval $(call add_share_file,share/yosys-syn/SRC/FF_MODELS,techlibs/yosys-syn/SRC/FF_MODELS/dffers.v))


# Z1010 DSP related
#
$(eval $(call add_share_file,share/yosys-syn/ARCHITECTURE/Z1010/DSP,techlibs/yosys-syn/ARCHITECTURE/Z1010/DSP/DSP48_sim.v))

$(eval $(call add_share_file,share/yosys-syn/ARCHITECTURE/Z1010/DSP,techlibs/yosys-syn/ARCHITECTURE/Z1010/DSP/mult18x18_DSP48.v))


OBJS += techlibs/yosys-syn/SRC/dsp.o

GENFILES += techlibs/yosys-syn/ARCHITECTURE/Z1010/DSP/dsp_CREG_pm.h
GENFILES += techlibs/yosys-syn/ARCHITECTURE/Z1010/DSP/dsp_cascade_pm.h

techlibs/yosys-syn/SRC/dsp.o: techlibs/yosys-syn/ARCHITECTURE/Z1010/DSP/dsp_CREG_pm.h techlibs/yosys-syn/ARCHITECTURE/Z1010/DSP/dsp_cascade_pm.h

$(eval $(call add_extra_objs,techlibs/yosys-syn/ARCHITECTURE/Z1010/DSP/dsp_CREG_pm.h))
$(eval $(call add_extra_objs,techlibs/yosys-syn/ARCHITECTURE/Z1010/DSP/dsp_cascade_pm.h))

