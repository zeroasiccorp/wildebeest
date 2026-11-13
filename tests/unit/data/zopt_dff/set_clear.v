// zopt_dff fixture: SET/CLR inputs that are constant (always / never active,
// per bit and per cell), on coarse and fine cells.  Read with -icells.
module top (input CLK, EN, SET, CLR, input [5:0] D, output [5:0] q0, q1, q2, q3, output [3:0] q4, q5, q6, output q7, q8, q9);
  // per bit: SET const 1 / 0 / x / wire ; CLR wire / const 1 / 0 / x
  $dffsr #(.CLK_POLARITY(1'b1), .SET_POLARITY(1'b1), .CLR_POLARITY(1'b1), .WIDTH(6)) ff0 (.CLK(CLK), .CLR({CLR, CLR, CLR, 1'b1, 1'b0, 1'bx}), .SET({1'b1, 1'b0, 1'bx, SET, SET, SET}), .D(D), .Q(q0));
  $dffsre #(.CLK_POLARITY(1'b1), .SET_POLARITY(1'b0), .CLR_POLARITY(1'b0), .EN_POLARITY(1'b1), .WIDTH(6)) ff1 (.CLK(CLK), .EN(EN), .CLR({CLR, CLR, CLR, 1'b1, 1'b0, 1'bx}), .SET({1'b1, 1'b0, 1'bx, SET, SET, SET}), .D(D), .Q(q1));
  $dlatchsr #(.SET_POLARITY(1'b0), .CLR_POLARITY(1'b1), .EN_POLARITY(1'b1), .WIDTH(6)) ff2 (.EN(EN), .CLR({CLR, CLR, CLR, 1'b1, 1'b0, 1'bx}), .SET({1'b1, 1'b0, 1'bx, SET, SET, SET}), .D(D), .Q(q2));
  $sr #(.SET_POLARITY(1'b1), .CLR_POLARITY(1'b0), .WIDTH(6)) ff3 (.CLR({CLR, CLR, CLR, 1'b1, 1'b0, 1'bx}), .SET({1'b1, 1'b0, 1'bx, SET, SET, SET}), .Q(q3));
  // CLR never active, SET same signal on all bits -> async reset to 1
  $dffsr #(.CLK_POLARITY(1'b1), .SET_POLARITY(1'b1), .CLR_POLARITY(1'b1), .WIDTH(4)) ff4 (.CLK(CLK), .CLR(4'b0000), .SET({SET, SET, SET, SET}), .D(D[3:0]), .Q(q4));
  // SET never active, CLR same signal -> async reset to 0
  $dffsr #(.CLK_POLARITY(1'b1), .SET_POLARITY(1'b0), .CLR_POLARITY(1'b0), .WIDTH(4)) ff5 (.CLK(CLK), .CLR({CLR, CLR, CLR, CLR}), .SET(4'b1111), .D(D[3:0]), .Q(q5));
  // SET and CLR on different bits but same signal and polarity -> async reset with mixed value
  $dffsr #(.CLK_POLARITY(1'b1), .SET_POLARITY(1'b1), .CLR_POLARITY(1'b1), .WIDTH(4)) ff6 (.CLK(CLK), .CLR({SET, 1'b0, SET, 1'b0}), .SET({1'b0, SET, 1'b0, SET}), .D(D[3:0]), .Q(q6));
  // fine cells: CLR always active (P polarity tied 1), SET always active (N tied 0)
  $_DFFSR_PPP_ ff7 (.C(CLK), .S(SET), .R(1'b1), .D(D[0]), .Q(q7));
  $_DFFSR_PNP_ ff8 (.C(CLK), .S(1'b0), .R(CLR), .D(D[1]), .Q(q8));
  $_SR_PP_ ff9 (.S(1'b0), .R(1'b0), .Q(q9));
endmodule
