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


	// wire [Y_WIDTH-1:0] OUT;

	// Don't specify clock or reset.
	MAE #(
		.BYPASS_A(1'b0),
		.BYPASS_B(1'b0),
		.BYPASS_C(1'b0),
		.BYPASS_P(1'b0),
		.POST_ADDER_STATIC(1'b0)
 	)
    _TECHMAP_REPLACE_ (
		.A(A),
		.A_BYPASS(1'b1),
		.B(B),
		.B_BYPASS(1'b1),
		.P(Y),
		.P_BYPASS(1'b1),
		.A_SRST_N(1'b1),
		.B_SRST_N(1'b1),
		.C_SRST_N(1'b1),
		.C_ARST_N(1'b1)
		// .C(Y_WIDTH'b0),// this being uncommented leads to regio not working? detects constant chunks
		// .C_BYPASS(1'b1),
		// .C_EN(1'b1), // dont include
	);
	// assign Y = OUT;
endmodule
