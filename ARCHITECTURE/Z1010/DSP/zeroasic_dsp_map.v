module \$__MUL18X18
  #(
	parameter A_SIGNED = 0,
	parameter B_SIGNED = 0,
	parameter A_WIDTH = 0,
	parameter B_WIDTH = 0,
	parameter Y_WIDTH = 0
  ) 
    (
		input [A_WIDTH-1:0] A,
		input [B_WIDTH-1:0] B,
		output [Y_WIDTH-1:0] Y
	);


	wire [Y_WIDTH-1:0] OUT;

	MAE _TECHMAP_REPLACE_ (
		.A(A),
		.A_BYPASS(1'b1),
		.B(B),
		.B_BYPASS(1'b1),
		.C(48'b0),
		.C_BYPASS(1'b1),
		.P(OUT),

		.A_SRST_N(1'b1),
		.B_SRST_N(1'b1),
		.C_SRST_N(1'b1),
		.C_ARST_N(1'b1),
		// todo:
		// clk
		// resetn ()
	);
	assign Y = OUT;
endmodule
