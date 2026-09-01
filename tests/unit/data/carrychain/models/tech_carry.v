//tech_carry.v
//
//Blackbox declaration of the hard carry-chain adder primitive targeted by
//data/z1010/tech_adder.v.  Kept out of the techmap file on purpose : any
//module in a techmap file is a mapping rule, so the primitive would be
//expanded back into soft logic if it lived there.

(* blackbox *)
module efpga_carry
  (
   input  a,
   input  b,
   input  ci,
   output s,
   output co
   );

   assign s  = a ^ b ^ ci;
   assign co = (a & b) | (a & ci) | (b & ci);

endmodule
