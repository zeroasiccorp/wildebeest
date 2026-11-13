// Small single-module design used by the synth_fpga option and flow tests:
// an 8-bit accumulator with async active-low reset and enable.
module top (
    input            clk,
    input            nreset,
    input            en,
    input      [7:0] d,
    output reg [7:0] q
);
   always @(posedge clk or negedge nreset)
      if (!nreset) q <= 8'h00;
      else if (en) q <= q + d;
endmodule
