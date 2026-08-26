//#############################################################################
// Copyright: Zero ASIC. All rights Reserved.
// Author: Andreas Olofsson
// License:  MIT (see LICENSE file in LogikBench repository)
//#############################################################################
//
// fmadd8: 8-bit (E4M3) fused multiply-add. It reuses the fmadd32 datapath
// wrapper, overriding its parameters to the E4M3 fp8 field layout (1 sign, 4
// exponent, 3 mantissa bits). See the fmadd32 block for the shared arithmetic.
//
//#############################################################################
module fmadd8
  (
   input [7:0]	a, // multiplicand (E4M3 fp8)
   input [7:0]	b, // multiplier
   input [7:0]	c, // addend
   output [7:0]	r  // round(a*b + c), fused
   );

   fmadd32 #(.EXP(4), .MANT(3)) u_fmadd32 (.a(a), .b(b), .c(c), .r(r));

endmodule
