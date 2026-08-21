//tech_adder.v
//
//Hard carry-chain adder tech mapping used by the tests/unit/carrychain
//unit tests.  It maps the $alu cells created by the 'alumacc' pass onto a
//bit-sliced hard carry-chain primitive ('efpga_carry') so that the
//arithmetic never reaches the generic '+/techmap.v' soft logic mapping.
//
//The primitive itself is deliberately NOT declared in this file : modules
//that live in a techmap file are themselves treated as mapping rules and
//would be expanded back into soft logic.  The blackbox declaration lives in
//models/tech_carry.v instead.

(* techmap_celltype = "$alu" *)
module \$__CARRYCHAIN_ALU (A, B, CI, BI, X, Y, CO);

   parameter A_SIGNED = 0;
   parameter B_SIGNED = 0;
   parameter A_WIDTH = 1;
   parameter B_WIDTH = 1;
   parameter Y_WIDTH = 1;

   (* force_downto *)
   input [(A_WIDTH-1):0] A;
   (* force_downto *)
   input [(B_WIDTH-1):0] B;

   input 		 CI, BI;

   (* force_downto *)
   output [(Y_WIDTH-1):0] X, Y;
   (* force_downto *)
   output [(Y_WIDTH-1):0] CO;

   //Sign/zero extend both operands up to the full result width.
   (* force_downto *)
   wire [(Y_WIDTH-1):0]   AA, BB;

   \$pos #(.A_SIGNED(A_SIGNED), .A_WIDTH(A_WIDTH), .Y_WIDTH(Y_WIDTH)) A_conv (.A(A), .Y(AA));
   \$pos #(.A_SIGNED(B_SIGNED), .A_WIDTH(B_WIDTH), .Y_WIDTH(Y_WIDTH)) B_conv (.A(B), .Y(BB));

   //'BI' asks for the B operand to be inverted (subtraction).
   (* force_downto *)
   wire [(Y_WIDTH-1):0]   BBB = BI ? ~BB : BB;

   //The 'X' output of $alu is the propagate signal.
   assign X = AA ^ BBB;

   //Carry chain : bit 'i' consumes the carry out of bit 'i-1'.
   (* force_downto *)
   wire [(Y_WIDTH-1):0]   C = {CO, CI};

   genvar 		  i;
   generate
      for (i = 0; i < Y_WIDTH; i = i + 1) begin:slice
	 efpga_carry carry (
			    .a(AA[i]),
			    .b(BBB[i]),
			    .ci(C[i]),
			    .s(Y[i]),
			    .co(CO[i])
			    );
      end
   endgenerate

endmodule
