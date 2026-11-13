// zopt_dff fixture: rarer constant-control and merge shapes.  Read with
// -icells, then run proc.
module top (input clk, en, l, r, s, c, input [3:0] d, ad,
            output [3:0] q_al_ar, q_al_sr, q_mixset, q_ce_dq, q_al_sat,
            output q_f_al_ar, q_f_ce_dq,
            output reg [3:0] q_srst_mux, q_part_srst);
  // always-active async load together with an async reset (coarse, fine)
  $aldff #(.CLK_POLARITY(1'b1), .ALOAD_POLARITY(1'b1), .WIDTH(4)) al_ar0 (.CLK(clk), .ALOAD(1'b1), .AD(ad), .D(d), .Q(q_al_ar));
  $_ALDFF_PP_ f_al_ar (.C(clk), .L(1'b1), .AD(ad[0]), .D(d[0]), .Q(q_f_al_ar));
  // always-active async load on a cell that also has set/clear
  $dffsr #(.CLK_POLARITY(1'b1), .SET_POLARITY(1'b1), .CLR_POLARITY(1'b0), .WIDTH(4)) al_sr (.CLK(clk), .SET({s, s, 1'b0, 1'b0}), .CLR({1'b1, 1'b1, c, c}), .D(d), .Q(q_al_sr));
  // CLR never active, SET on different signals per bit -> cannot become ARST
  $dffsr #(.CLK_POLARITY(1'b1), .SET_POLARITY(1'b1), .CLR_POLARITY(1'b1), .WIDTH(4)) mixset (.CLK(clk), .CLR(4'b0000), .SET({s, s, c, c}), .D(d), .Q(q_mixset));
  // $sdffce with D == Q and an active-low enable
  $sdffce #(.CLK_POLARITY(1'b1), .SRST_POLARITY(1'b0), .SRST_VALUE(4'b0110), .EN_POLARITY(1'b0), .WIDTH(4)) ce_dq (.CLK(clk), .SRST(r), .EN(en), .D(q_ce_dq), .Q(q_ce_dq));
  $_SDFFCE_PP0N_ f_ce_dq (.C(clk), .R(r), .E(en), .D(q_f_ce_dq), .Q(q_f_ce_dq));
  // async-load register with a non-constant AD under -sat
  (* init = 4'b0000 *) wire [3:0] q_al_sat;
  $aldff #(.CLK_POLARITY(1'b1), .ALOAD_POLARITY(1'b1), .WIDTH(4)) al_sat (.CLK(clk), .ALOAD(l), .AD(ad & d), .D(q_al_sat & d), .Q(q_al_sat));
  // sync reset to a constant with both mux inputs constant on some bits
  always @(posedge clk) if (r) q_srst_mux <= 4'b0101; else q_srst_mux <= {d[3:2], 2'b11};
  // sync reset that only covers part of the bits
  always @(posedge clk) begin
    q_part_srst[1:0] <= r ? 2'b00 : d[1:0];
    q_part_srst[3:2] <= d[3:2];
  end
endmodule
