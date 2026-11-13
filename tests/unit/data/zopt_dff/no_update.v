// zopt_dff fixture: flip-flops that never load D (enable never active,
// constant clock), with defined init values.  Read with -icells.
module top (input CLK, EN, R, input [3:0] D, output [3:0] q0, q1, q4, output q3);
  (* init = 4'b1010 *) wire [3:0] q0;
  (* init = 4'b0011 *) wire [3:0] q1;
  (* init = 4'b0110 *) wire [3:0] q4;
  (* init = 1'b1 *) wire q3;
  $dffe #(.CLK_POLARITY(1'b1), .EN_POLARITY(1'b1), .WIDTH(4)) en_never (.CLK(CLK), .EN(1'b0), .D(D), .Q(q0));
  $dff #(.CLK_POLARITY(1'b1), .WIDTH(4)) clk_const (.CLK(1'b0), .D(D), .Q(q1));
  $sdffe #(.CLK_POLARITY(1'b1), .SRST_POLARITY(1'b1), .SRST_VALUE(4'b1111), .EN_POLARITY(1'b1), .WIDTH(4)) en_never_undef (.CLK(CLK), .SRST(R), .EN(1'b0), .D(D), .Q(q4));
  $_DFFE_PN_ f_en_never (.C(CLK), .E(1'b1), .D(D[0]), .Q(q3));
endmodule
