/*
ISC License

Copyright (C) 2024 Microchip Technology Inc. and its subsidiaries

Permission to use, copy, modify, and/or distribute this software for any
purpose with or without fee is hereby granted, provided that the above
copyright notice and this permission notice appear in all copies.

THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
*/

module \$__MUL18X18 (input [17:0] A, input [17:0] B, output [39:0] Y);
	parameter A_SIGNED = 0;
	parameter B_SIGNED = 0;
	parameter A_WIDTH = 0;
	parameter B_WIDTH = 0;
	parameter Y_WIDTH = 0;

	wire [39:0] OUT;

	MAE _TECHMAP_REPLACE_ (
		// keep
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
