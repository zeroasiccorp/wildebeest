// zopt_dff -sat fixture: registers that can or cannot be proven to stay at
// their init value.  Run proc first.
module top (input clk, a, b, c, rst, (* onehot *) input [3:0] sel, input [7:0] w,
            output reg q0, q1, q2, q3, q4, q5, q6, q8, q9, q10, output reg [1:0] q7);
  initial q0 = 0;            // q0 <= q0 & a : stays 0 -> SAT proves constant
  always @(posedge clk) q0 <= q0 & a;
  initial q1 = 0;            // q1 <= q1 | a : can become 1 -> SAT counterexample
  always @(posedge clk) q1 <= q1 | a;
  initial q2 = 1;            // D straight from an input
  always @(posedge clk) q2 <= a;
  always @(posedge clk) q3 <= q3 & b;   // no init -> skipped
  // deep cone (> 5 levels) feeding a stuck register
  initial q4 = 0;
  wire t1 = a ^ b, t2 = t1 ^ c, t3 = t2 & w[0], t4 = t3 | w[1], t5 = t4 ^ w[2], t6 = t5 & w[3], t7 = t6 | w[4];
  always @(posedge clk) q4 <= q4 & t7;
  // register sharing the cone of q0
  initial q5 = 0;
  always @(posedge clk) q5 <= q0 & q5 & b;
  // sel is one-hot: two of its bits are never 1 together, so q6 stays 0
  // only if the onehot attribute is taken into account
  initial q6 = 0;
  always @(posedge clk) q6 <= sel[0] & sel[1];
  // 2-bit register: one bit stuck, one free -> partial slice
  initial q7 = 2'b00;
  always @(posedge clk) q7 <= {q7[1] & a, q7[0] | b};
  // cones with an adder, a shifter and a multiplier (different SAT
  // complexity classes)
  initial q8 = 0;
  wire [3:0] sum = w[3:0] + sel;
  always @(posedge clk) q8 <= q8 & sum[2];
  initial q9 = 0;
  wire [7:0] shl = w << sel[1:0];
  always @(posedge clk) q9 <= q9 & shl[3];
  initial q10 = 0;
  wire [7:0] prod = w[3:0] * sel;
  always @(posedge clk) q10 <= q10 & prod[1];
endmodule
