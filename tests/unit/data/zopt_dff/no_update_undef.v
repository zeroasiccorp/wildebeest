// zopt_dff fixture: a flip-flop with a constant clock and a partly undefined
// init value.  Read with -icells.
module top (input CLK, input [3:0] D, output [3:0] Q);
  (* init = 4'b1x0x *) wire [3:0] Q;
  $dff #(.CLK_POLARITY(1'b1), .WIDTH(4)) clk_const_undef (.CLK(1'b1), .D(D), .Q(Q));
endmodule
