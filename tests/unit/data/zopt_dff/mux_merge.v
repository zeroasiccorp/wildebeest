// zopt_dff fixture: reset / enable muxes in front of flip-flops, to be
// folded into srst / enable inputs.  Run proc first.
module top (input clk, en, en2, rst, rst2, input [3:0] d, output reg [3:0] q0, q1, q2, q3, q4, q5, q6);
  always @(posedge clk) if (rst) q0 <= 4'b1010; else q0 <= d;
  always @(posedge clk) if (en) q1 <= d;
  always @(posedge clk) if (rst) q2 <= 0; else if (en) q2 <= d;
  always @(posedge clk) if (en) begin if (rst) q3 <= 4'hf; else q3 <= d; end
  always @(posedge clk) if (!rst) q4 <= 0; else if (!rst2) q4 <= 4'h3; else q4 <= d;
  always @(posedge clk) if (en) q5 <= d; else if (en2) q5 <= ~d;
  always @(posedge clk) case ({en, en2}) 2'b01: q6 <= d; 2'b10: q6 <= ~d; 2'b11: q6 <= 4'h5; default: ; endcase
endmodule
