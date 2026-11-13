// An XOR tree with a repeated leaf: (a8 ^ a9) ^ (a8 ^ a10) ^ (a11 ^ a12),
// which equals a9 ^ a10 ^ a11 ^ a12 because a8 cancels out.
module top (input [12:8] a, output p3);
   wire t1 = a[8] ^ a[9];
   wire t2 = a[8] ^ a[10];
   wire t3 = a[11] ^ a[12];
   assign p3 = t1 ^ t2 ^ t3;
endmodule
