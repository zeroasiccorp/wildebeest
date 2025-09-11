module \$__MAE__
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

	// Don't specify clock or reset.
	MAE #(
		.BYPASS_A(1'b0),
		.BYPASS_B(1'b0),
		.BYPASS_C(1'b0),
		.BYPASS_P(1'b0),
		.POST_ADDER_STATIC(1'b0),
		.USE_FEEDBACK(1'b0)
 	)
    _TECHMAP_REPLACE_ (
		.A(A),
		.A_BYPASS(1'b1),
		.B(B),
		.B_BYPASS(1'b1),
		.C(40'b0),
		.P(Y),
		.P_BYPASS(1'b1),
		.A_ARST_N(1'b1),
		.B_ARST_N(1'b1),
		.C_ARST_N(1'b1),
		.P_ARST_N(1'b1)

	);
endmodule
