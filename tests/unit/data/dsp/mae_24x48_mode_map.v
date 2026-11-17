// A package-supplied DSP mode map for a 24x24 -> 48 block, used through the
// config file key 'dsps.mode_techmap'.
//
// It declares every parameter a 'MAE' cell can carry, because 'techmap' fails
// on a cell parameter the template does not declare. Only the plain multiply
// is mapped, onto a cell type the plugin's own z1010 map cannot produce.
module MAE #(
  parameter A_REG=0,
  parameter B_REG=0,
  parameter C_REG=0,
  parameter P_REG=0,
  parameter MULT_HAS_REG=0,
  parameter POST_ADDER_STATIC=0,
  parameter USE_FEEDBACK=0,
  parameter ADD_ONLY=0
 )
(
  input [23:0] A,
  input ALLOW_A_REG,
  input A_EN,
  input A_ARST_N,
  input [23:0] B,
  input ALLOW_B_REG,
  input CDIN_FDBK_SEL,
  input B_EN,
  input B_ARST_N,
  input [47:0] C,
  input CLK,
  input C_ARST_N,
  input ALLOW_C_REG,
  input C_EN,
  output [47:0] P,
  input ALLOW_P_REG,
  input P_EN,
  input P_ARST_N,
  input resetn
);

generate
  if (ADD_ONLY == 1'b0 && A_REG == 1'b0 && B_REG == 1'b0 && C_REG == 1'b0 &&
      P_REG == 1'b0 && POST_ADDER_STATIC == 1'b0 && USE_FEEDBACK == 1'b0) begin
    efpga_mult_24x24 _TECHMAP_REPLACE_ (
      .a(A),
      .b(B),
      .y(P)
    );
  end
  else
  begin
    wire _TECHMAP_FAIL_ = 1'b1;
  end
endgenerate
endmodule
