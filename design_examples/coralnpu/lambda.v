// SRAM black boxes referenced by coralnpu.sv (Chisel emits the module names
// Sram_512x128 / Sram_2048x128 but leaves the implementation to the target
// technology library). These map coralnpu's 16-bit byte mask onto lambdalib
// la_spram running in byte mode (BYTEMASK=1), which takes a DW/8-wide
// per-byte write mask and maps to byte-wide FPGA BRAM.

module Sram_512x128(
  input          clock,
  input          enable,
  input          write,
  input  [8:0]   addr,
  input  [127:0] wdata,
  input  [15:0] wmask,
  output [127:0] rdata
);

  la_spram #(.DW(128), .AW(9), .BYTEMASK(1)) memory (
    .clk(clock),
    .ce(enable),
    .we(write),
    .wmask(wmask),
    .addr(addr),
    .din(wdata),
    .dout(rdata),
    .selctrl(1'b0),
    .ctrl('b0),
    .status()
  );

endmodule

module Sram_2048x128(
  input          clock,
  input          enable,
  input          write,
  input  [10:0]   addr,
  input  [127:0] wdata,
  input  [15:0] wmask,
  output [127:0] rdata
);

  la_spram #(.DW(128), .AW(11), .BYTEMASK(1)) memory (
    .clk(clock),
    .ce(enable),
    .we(write),
    .wmask(wmask),
    .addr(addr),
    .din(wdata),
    .dout(rdata),
    .selctrl(1'b0),
    .ctrl('b0),
    .status()
  );

endmodule
