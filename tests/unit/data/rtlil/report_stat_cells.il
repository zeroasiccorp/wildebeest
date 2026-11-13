# Fixture for the report_stat cell-count test (generated once, kept as-is).
# One cell of every library cell type report_stat counts, plus the four
# yosys internal types it counts ($lut, $not, $_DFF*, $_SDFF*).
#
# LUTs : 19 cells, counted as 29 (mux4x0 = 3, mux8x0 = 7, plus $lut and $not)
# DFFs : 34 cells + $_DFF_P_ + $_SDFF_PP0_ = 36
# DSPs : 23
# BRAMs: 42
module \top
  wire input 1 \a
  wire input 2 \clk
  wire output 3 \y
  wire output 4 \n
  wire output 5 \q1
  wire output 6 \q2
  cell $lut \int_lut
    parameter \WIDTH 1
    parameter \LUT 2'10
    connect \A \a
    connect \Y \y
  end
  cell $not \int_not
    parameter \A_SIGNED 0
    parameter \A_WIDTH 1
    parameter \Y_WIDTH 1
    connect \A \a
    connect \Y \n
  end
  cell $_DFF_P_ \int_dff
    connect \C \clk
    connect \D \a
    connect \Q \q1
  end
  cell $_SDFF_PP0_ \int_sdff
    connect \C \clk
    connect \R \a
    connect \D \a
    connect \Q \q2
  end
  cell \LUT1 \c_LUT1
  end
  cell \LUT2 \c_LUT2
  end
  cell \LUT3 \c_LUT3
  end
  cell \LUT4 \c_LUT4
  end
  cell \INV \c_INV
  end
  cell \MUXF5 \c_MUXF5
  end
  cell \MUXF6 \c_MUXF6
  end
  cell \MUXF7 \c_MUXF7
  end
  cell \LUT5 \c_LUT5
  end
  cell \LUT6 \c_LUT6
  end
  cell \SB_LUT4 \c_SB_LUT4
  end
  cell \SB_CARRY \c_SB_CARRY
  end
  cell \mux4x0 \c_mux4x0
  end
  cell \mux8x0 \c_mux8x0
  end
  cell \CFG1 \c_CFG1
  end
  cell \CFG2 \c_CFG2
  end
  cell \CFG3 \c_CFG3
  end
  cell \CFG4 \c_CFG4
  end
  cell \cycloneiv_lcell_comb \c_cycloneiv_lcell_comb
  end
  cell \dffer \c_dffer
  end
  cell \dffes \c_dffes
  end
  cell \dffe \c_dffe
  end
  cell \dffr \c_dffr
  end
  cell \dffs \c_dffs
  end
  cell \dff \c_dff
  end
  cell \dffh \c_dffh
  end
  cell \dffeh \c_dffeh
  end
  cell \dffl \c_dffl
  end
  cell \dffel \c_dffel
  end
  cell \dffhl \c_dffhl
  end
  cell \dffehl \c_dffehl
  end
  cell \FDCE \c_FDCE
  end
  cell \FDPE \c_FDPE
  end
  cell \FDRE \c_FDRE
  end
  cell \FDRE_1 \c_FDRE_1
  end
  cell \FDSE \c_FDSE
  end
  cell \LDCE \c_LDCE
  end
  cell \TRELLIS_FF \c_TRELLIS_FF
  end
  cell \SB_DFF \c_SB_DFF
  end
  cell \SB_DFFE \c_SB_DFFE
  end
  cell \SB_DFFER \c_SB_DFFER
  end
  cell \SB_DFFESR \c_SB_DFFESR
  end
  cell \SB_DFFESS \c_SB_DFFESS
  end
  cell \SB_DFFN \c_SB_DFFN
  end
  cell \SB_DFFR \c_SB_DFFR
  end
  cell \SB_DFFS \c_SB_DFFS
  end
  cell \SB_DFFSR \c_SB_DFFSR
  end
  cell \SB_DFFES \c_SB_DFFES
  end
  cell \dffepc \c_dffepc
  end
  cell \SLE \c_SLE
  end
  cell \dffeas \c_dffeas
  end
  cell \dffer_pxp \c_dffer_pxp
  end
  cell \dffes_xpp \c_dffes_xpp
  end
  cell \DSP48 \c_DSP48
  end
  cell \DSP48E \c_DSP48E
  end
  cell \DSP48E1 \c_DSP48E1
  end
  cell \SB_MAC16 \c_SB_MAC16
  end
  cell \MACC_PA \c_MACC_PA
  end
  cell \efpga_adder \c_efpga_adder
  end
  cell \efpga_adder_regi \c_efpga_adder_regi
  end
  cell \efpga_adder_rego \c_efpga_adder_rego
  end
  cell \efpga_adder_regio \c_efpga_adder_regio
  end
  cell \efpga_acc_regi \c_efpga_acc_regi
  end
  cell \efpga_acc \c_efpga_acc
  end
  cell \efpga_mult \c_efpga_mult
  end
  cell \efpga_mult_regi \c_efpga_mult_regi
  end
  cell \efpga_mult_rego \c_efpga_mult_rego
  end
  cell \efpga_mult_regio \c_efpga_mult_regio
  end
  cell \efpga_mult_addc_regi \c_efpga_mult_addc_regi
  end
  cell \efpga_mult_addc_rego \c_efpga_mult_addc_rego
  end
  cell \efpga_mult_addc_regio \c_efpga_mult_addc_regio
  end
  cell \efpga_mult_addc \c_efpga_mult_addc
  end
  cell \efpga_macc_pipe \c_efpga_macc_pipe
  end
  cell \efpga_macc_pipe_regi \c_efpga_macc_pipe_regi
  end
  cell \efpga_macc_regi \c_efpga_macc_regi
  end
  cell \efpga_macc \c_efpga_macc
  end
  cell \RAMB16 \c_RAMB16
  end
  cell \RAM32M \c_RAM32M
  end
  cell \SB_RAM40_4K \c_SB_RAM40_4K
  end
  cell \DP8KC \c_DP8KC
  end
  cell \altsyncram \c_altsyncram
  end
  cell \RAM1K20 \c_RAM1K20
  end
  cell \RAM64x12 \c_RAM64x12
  end
  cell \spram_512x32 \c_spram_512x32
  end
  cell \spram_1024x16 \c_spram_1024x16
  end
  cell \spram_2048x8 \c_spram_2048x8
  end
  cell \spram_4096x4 \c_spram_4096x4
  end
  cell \spram_8192x2 \c_spram_8192x2
  end
  cell \spram_16384x1 \c_spram_16384x1
  end
  cell \spram_512x64 \c_spram_512x64
  end
  cell \spram_1024x32 \c_spram_1024x32
  end
  cell \spram_2048x16 \c_spram_2048x16
  end
  cell \spram_4096x8 \c_spram_4096x8
  end
  cell \spram_8192x4 \c_spram_8192x4
  end
  cell \spram_16384x2 \c_spram_16384x2
  end
  cell \spram_32768x1 \c_spram_32768x1
  end
  cell \sdpram_1024x16 \c_sdpram_1024x16
  end
  cell \sdpram_2048x8 \c_sdpram_2048x8
  end
  cell \sdpram_4096x4 \c_sdpram_4096x4
  end
  cell \sdpram_8192x2 \c_sdpram_8192x2
  end
  cell \sdpram_16384x1 \c_sdpram_16384x1
  end
  cell \sdpram_1024x32 \c_sdpram_1024x32
  end
  cell \sdpram_2048x16 \c_sdpram_2048x16
  end
  cell \sdpram_4096x8 \c_sdpram_4096x8
  end
  cell \sdpram_8192x4 \c_sdpram_8192x4
  end
  cell \sdpram_16384x2 \c_sdpram_16384x2
  end
  cell \sdpram_32768x1 \c_sdpram_32768x1
  end
  cell \tdpram_1024x16 \c_tdpram_1024x16
  end
  cell \tdpram_2048x8 \c_tdpram_2048x8
  end
  cell \tdpram_4096x4 \c_tdpram_4096x4
  end
  cell \tdpram_8192x2 \c_tdpram_8192x2
  end
  cell \tdpram_16384x1 \c_tdpram_16384x1
  end
  cell \tdpram_1024x32 \c_tdpram_1024x32
  end
  cell \tdpram_2048x16 \c_tdpram_2048x16
  end
  cell \tdpram_4096x8 \c_tdpram_4096x8
  end
  cell \tdpram_8192x4 \c_tdpram_8192x4
  end
  cell \tdpram_16384x2 \c_tdpram_16384x2
  end
  cell \tdpram_32768x1 \c_tdpram_32768x1
  end
end
