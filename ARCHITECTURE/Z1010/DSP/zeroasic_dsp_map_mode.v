module MAE #(
  parameter BYPASS_A=0,
  parameter BYPASS_B=0,
  parameter BYPASS_C=0,
  parameter BYPASS_P=0,
  parameter POST_ADDER_STATIC=0
 )
(
  input [17:0] A,
  input A_BYPASS,
  input A_EN, // do not remove
  input A_SRST_N,
  input [17:0] B,
  input B_BYPASS,
  input B_EN,  // do not remove
  input B_SRST_N,
  input [39:0] C,
  input CLK,
  input C_ARST_N,
  input C_BYPASS, //new!
  input C_EN,
  input C_SRST_N, // if you remove this it break efpga_mult_regi...
  input [47:0] P,
  input P_BYPASS,
  input P_EN,  // do not remove
  input P_SRST_N
);

generate
  if (BYPASS_A == 1'b1 && BYPASS_B == 1'b1 && BYPASS_P==1'b0 && POST_ADDER_STATIC==1'b0) begin
    efpga_mult_regi _TECHMAP_REPLACE_ (
      .A(A),
      .B(B),
      .Y(P)
    );
  end
  else if (BYPASS_A == 1'b0 && BYPASS_B == 1'b0 && BYPASS_P==1'b1 && POST_ADDER_STATIC==1'b0) begin
    efpga_mult_rego _TECHMAP_REPLACE_ (
      .A(A),
      .B(B),
      .Y(P)
    );
  end
  else if (BYPASS_A == 1'b1 && BYPASS_B == 1'b1 && BYPASS_P==1'b1 && POST_ADDER_STATIC==1'b0) begin
    efpga_mult_regio _TECHMAP_REPLACE_ (
      .A(A),
      .B(B),
      .Y(P)
    );
  end
  else if (BYPASS_A == 1'b0 && BYPASS_B == 1'b0 && BYPASS_P==1'b0 && POST_ADDER_STATIC==1'b0) begin
    efpga_mult _TECHMAP_REPLACE_ (
      .A(A),
      .B(B),
      .Y(P)
    );
  end
  if (BYPASS_A == 1'b1 && BYPASS_B == 1'b1 && BYPASS_P==1'b0 && POST_ADDER_STATIC==1'b1) begin
    efpga_mult_addc_regi _TECHMAP_REPLACE_ (
      .A(A),
      .B(B),
      .Y(P)
    );
  end
  else if (BYPASS_A == 1'b0 && BYPASS_B == 1'b0 && BYPASS_P==1'b1 && POST_ADDER_STATIC==1'b1) begin
    efpga_mult_addc_rego _TECHMAP_REPLACE_ (
      .A(A),
      .B(B),
      .Y(P)
    );
  end
  else if (BYPASS_A == 1'b1 && BYPASS_B == 1'b1 && BYPASS_P==1'b1 && POST_ADDER_STATIC==1'b1) begin
    efpga_mult_addc_regio _TECHMAP_REPLACE_ (
      .A(A),
      .B(B),
      .Y(P)
    );
  end
  else if (BYPASS_A == 1'b0 && BYPASS_B == 1'b0 && BYPASS_P==1'b0 && POST_ADDER_STATIC==1'b1) begin
    efpga_mult_addc _TECHMAP_REPLACE_ (
      .A(A),
      .B(B),
      .Y(P)
    );
  end
  else if (BYPASS_A == 1'b0 && BYPASS_B == 1'b0 && BYPASS_P==1'b0 && POST_ADDER_STATIC==1'b1) begin
    efpga_macc_pipe _TECHMAP_REPLACE_ (
      .A(A),
      .B(B),
      .Y(P)
    );
  end
  else if (BYPASS_A == 1'b1 && BYPASS_B == 1'b1 && BYPASS_P==1'b1 && POST_ADDER_STATIC==1'b1) begin
    efpga_macc_pipe_regi _TECHMAP_REPLACE_ (
      .A(A),
      .B(B),
      .Y(P)
    );
  end
  else if (BYPASS_A == 1'b1 && BYPASS_B == 1'b1 && BYPASS_P==1'b1 && POST_ADDER_STATIC==1'b1) begin
    efpga_macc_regi _TECHMAP_REPLACE_ (
      .A(A),
      .B(B),
      .Y(P)
    );
  end
  else if (BYPASS_A == 1'b0 && BYPASS_B == 1'b0 && BYPASS_P==1'b0 && POST_ADDER_STATIC==1'b0) begin
    efpga_macc _TECHMAP_REPLACE_ (
      .A(A),
      .B(B),
      // .C(C),
      .Y(P)
    );
  end
  else begin
    last_resort _TECHMAP_REPLACE_ (
      .A(A),
      .B(B),
      // .C(C),
      .Y(P)
    );
  end
endgenerate
endmodule