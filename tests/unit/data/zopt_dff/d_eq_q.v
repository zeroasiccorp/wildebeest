// zopt_dff fixture: flip-flops whose D is (partly) their own Q.
// Read with -icells.
module top (input CLK, EN, R, input [3:0] D, output [3:0] q0, q1, q2, q3, output q4, q5);
  $dff #(.CLK_POLARITY(1'b1), .WIDTH(4)) dq (.CLK(CLK), .D({q0[3:2], D[1:0]}), .Q(q0));
  $sdff #(.CLK_POLARITY(1'b1), .SRST_POLARITY(1'b1), .SRST_VALUE(4'b0101), .WIDTH(4)) dq_srst (.CLK(CLK), .SRST(R), .D(q1), .Q(q1));
  $sdffe #(.CLK_POLARITY(1'b1), .SRST_POLARITY(1'b0), .SRST_VALUE(4'b0101), .EN_POLARITY(1'b1), .WIDTH(4)) dq_srst_en (.CLK(CLK), .SRST(R), .EN(EN), .D(q2), .Q(q2));
  $adff #(.CLK_POLARITY(1'b1), .ARST_POLARITY(1'b1), .ARST_VALUE(4'b0101), .WIDTH(4)) dq_arst (.CLK(CLK), .ARST(R), .D(q3), .Q(q3));
  $_SDFF_PP0_ f_dq_srst (.C(CLK), .R(R), .D(q4), .Q(q4));
  $_SDFFE_PN1N_ f_dq_srst_en (.C(CLK), .R(R), .E(EN), .D(q5), .Q(q5));
endmodule
