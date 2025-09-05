module MAE #(
  parameter BYPASS_A=0,
  parameter BYPASS_B=0,
  parameter BYPASS_C=0,
  parameter BYPASS_P=0
 )
(
  input [17:0] A,
  input A_BYPASS,
  input A_EN,
  input A_SRST_N,
  input [17:0] B,
  input B_BYPASS,
  input B_EN,
  input B_SRST_N,
  input CLK,
  input C_ARST_N,
  input C_SRST_N,
  input [39:0] P,
  input P_BYPASS,
  input P_EN,
  input P_SRST_N
);

generate
  if (BYPASS_A == 1'b0 && BYPASS_B == 1'b0) begin
    efpga_mult_regi _TECHMAP_REPLACE_ (
      .A(A),
      .B(B),
      .mae_config(6'b100110),
      .Y(P)
    );
  end
  else begin
    fpga_mult_rego _TECHMAP_REPLACE_ (
      .A(A),
      .B(B),
      .mae_config(6'b100110),
      .Y(P)
    );
  end
endgenerate
endmodule