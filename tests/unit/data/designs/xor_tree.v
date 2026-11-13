// XOR-heavy combinational design for the '-opt delay' XOR-tree balancing
// tests: a 33-bit parity, a CRC-16 style next-state function, two parity
// trees that share inputs, and two that share a leaf pair (so the balanced
// trees can contain duplicate XORs).  All leaves within one tree are
// distinct (see xor_repeat.v for the repeated-leaf case).
module top (
    input      [32:0] a,
    input      [15:0] crc_in,
    input      [7:0]  data,
    output            parity,
    output     [15:0] crc_out,
    output            p1,
    output            p2,
    output            p4,
    output            p5
);
   assign parity = ^a;

   // CRC-16 (x^16 + x^15 + x^2 + 1), one byte at a time.
   function [15:0] crc16_step(input [15:0] c, input [7:0] d);
      integer i;
      reg [15:0] r;
      begin
         r = c;
         for (i = 0; i < 8; i = i + 1)
            r = (r[15] ^ d[i]) ? {r[14:0], 1'b0} ^ 16'h8005 : {r[14:0], 1'b0};
         crc16_step = r;
      end
   endfunction
   assign crc_out = crc16_step(crc_in, data);

   // overlapping parity trees: (a0^a1^a2^a3^a4) and (a3^a4^a5^a6^a7)
   assign p1 = a[0] ^ a[1] ^ a[2] ^ a[3] ^ a[4];
   assign p2 = a[3] ^ a[4] ^ a[5] ^ a[6] ^ a[7];

   // trees sharing the leaf pair (a20, a21)
   assign p4 = a[20] ^ a[21] ^ a[22] ^ a[23];
   assign p5 = a[20] ^ a[21] ^ a[24] ^ a[25];
endmodule
