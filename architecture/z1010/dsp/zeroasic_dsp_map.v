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
	generate
	if(A_SIGNED == 0 && B_SIGNED == 0) begin
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
	end else begin
      // For signed multiplications, use a generic software-based multiplier
      // that Yosys will break down into basic gates.
      $__soft_mul #(.A_WIDTH(A_WIDTH), .B_WIDTH(B_WIDTH), .Y_WIDTH(Y_WIDTH), .A_SIGNED(A_SIGNED), .B_SIGNED(B_SIGNED))
      soft_mult_inst (.A(A), .B(B), .Y(Y));
    end
	endgenerate
endmodule
