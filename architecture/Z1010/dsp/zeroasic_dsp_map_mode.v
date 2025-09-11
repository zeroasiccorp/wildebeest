module MAE #(
  parameter BYPASS_A=0,
  parameter BYPASS_B=0,
  parameter BYPASS_C=0,
  parameter BYPASS_P=0,
  parameter MULT_HAS_REG=0,
  parameter POST_ADDER_STATIC=0,
  parameter USE_FEEDBACK=0
 )
(
  input [17:0] A,
  input A_BYPASS,
  input A_EN,
  input A_SRST_N,
  input [17:0] B,
  input B_BYPASS,
  input CDIN_FDBK_SEL,
  input B_EN,
  input B_SRST_N,
  input [39:0] C,
  input CLK,
  input C_ARST_N,
  input C_BYPASS,
  input C_EN,
  input C_SRST_N,
  input [39:0] P,
  input P_BYPASS,
  input P_EN,
  input P_SRST_N
);

generate
  if (BYPASS_A == 1'b1 && BYPASS_B == 1'b1 && BYPASS_P==1'b0 && POST_ADDER_STATIC==1'b0) begin
    efpga_mult_regi _TECHMAP_REPLACE_ (
      .A(A),
      .B(B),
      .CLK(CLK),
      .Y(P)
    );
  end
  else if (BYPASS_A == 1'b0 && BYPASS_B == 1'b0 && BYPASS_P==1'b1 && POST_ADDER_STATIC==1'b0 && USE_FEEDBACK==1'b0) begin
    efpga_mult_rego _TECHMAP_REPLACE_ (
      .A(A),
      .B(B),
      .CLK(CLK),
      .Y(P)
    );
  end
  else if (BYPASS_A == 1'b1 && BYPASS_B == 1'b1 && BYPASS_P==1'b1 && POST_ADDER_STATIC==1'b0 && USE_FEEDBACK==1'b0) begin
    efpga_mult_regio _TECHMAP_REPLACE_ (
      .A(A),
      .B(B),
      .CLK(CLK),
      .Y(P)
    );
  end
  else if (BYPASS_A == 1'h0 && BYPASS_B == 1'h0 && BYPASS_C==1'h0 && BYPASS_P==1'h0 && POST_ADDER_STATIC==1'h0 && USE_FEEDBACK==1'h0) begin
    efpga_mult _TECHMAP_REPLACE_ (
      .A(A),
      .B(B),
      .Y(P)
    );
  end
  else if (BYPASS_A == 1'b1 && BYPASS_B == 1'b1 && BYPASS_P==1'b0 && POST_ADDER_STATIC==1'b1 && USE_FEEDBACK==1'b0) begin
    efpga_mult_addc_regi _TECHMAP_REPLACE_ (
      .A(A),
      .B(B),
      .C(C),
      .CLK(CLK),
      .Y(P)
    );
  end
  else if (BYPASS_A == 1'b0 && BYPASS_B == 1'b0 && BYPASS_P==1'b1 && POST_ADDER_STATIC==1'b1 && USE_FEEDBACK == 1'b0) begin
    efpga_mult_addc_rego _TECHMAP_REPLACE_ (
      .A(A),
      .B(B),
      .C(C),
      .Y(P)
    );
  end
  else if (BYPASS_A == 1'b1 && BYPASS_B == 1'b1 && BYPASS_C==1'b0 && BYPASS_P==1'b1 && POST_ADDER_STATIC==1'b1 && USE_FEEDBACK==1'b0) begin
    efpga_mult_addc_regio _TECHMAP_REPLACE_ (
      .A(A),
      .B(B),
      .C(C),
      .CLK(CLK),
      .Y(P)
    );
  end
  else if (BYPASS_A == 1'b0 && BYPASS_B == 1'b0 && BYPASS_P==1'b0 && POST_ADDER_STATIC==1'b1 && USE_FEEDBACK==1'b0) begin
    efpga_mult_addc _TECHMAP_REPLACE_ (
      .A(A),
      .B(B),
      .C(C),
      .Y(P)
    );
  end
  else if (BYPASS_A == 1'b0 && BYPASS_B == 1'b0 && BYPASS_P==1'b1 && POST_ADDER_STATIC==1'b1 && USE_FEEDBACK==1'b1 && MULT_HAS_REG == 1'b1) begin
    efpga_macc_pipe _TECHMAP_REPLACE_ (
      .A(A),
      .B(B),
      .CLK(CLK),
      .Y(P)
    );
  end
  else if (BYPASS_A == 1'b1 && BYPASS_B == 1'b1 && BYPASS_P==1'b1 && POST_ADDER_STATIC==1'b1 && USE_FEEDBACK == 1'b1 && MULT_HAS_REG == 1'b1) begin
    efpga_macc_pipe_regi _TECHMAP_REPLACE_ (
      .A(A),
      .B(B),
      .CLK(CLK),
      .Y(P)
    );
  end
  else if (BYPASS_A == 1'b1 && BYPASS_B == 1'b1 && BYPASS_P==1'b1 && POST_ADDER_STATIC==1'b1 && USE_FEEDBACK == 1'b1 && MULT_HAS_REG == 1'b0) begin
    efpga_macc_regi _TECHMAP_REPLACE_ (
      .A(A),
      .B(B),
      .CLK(CLK),
      .Y(P)
    );
  end
  else if (BYPASS_A == 1'b0 && BYPASS_B == 1'b0 && BYPASS_P==1'b1 && POST_ADDER_STATIC==1'b1 && USE_FEEDBACK == 1'b1 && MULT_HAS_REG == 1'b0) begin
    efpga_macc _TECHMAP_REPLACE_ (
      .A(A),
      .B(B),
      .CLK(CLK),
      .Y(P)
    );
  end
  else 
  begin
    wire _TECHMAP_FAIL_ = 1'b1;
  end
endgenerate
endmodule