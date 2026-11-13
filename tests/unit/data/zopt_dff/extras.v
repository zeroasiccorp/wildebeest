// zopt_dff fixture: less common shapes.  Read with -icells, then run proc.
module top (input clk, en, en2, r1, r2, l, input [1:0] s2, input [3:0] d, ad,
            output reg [3:0] q_pmux, q_inv_en, q_has_en, q_chain, q_sel_path,
            output [3:0] q_al_arst, q_al_sr, q_sdffce_dq, q_aldff_sat);
  // zero-width flip-flop
  $dff #(.CLK_POLARITY(1'b1), .WIDTH(0)) w0 (.CLK(clk), .D(), .Q());

  // feedback through a $pmux: hold for one select value
  always @(posedge clk)
    case (s2)
      2'd0: q_pmux <= d;
      2'd1: q_pmux <= q_pmux;
      2'd2: q_pmux <= ~d;
      default: q_pmux <= ad;
    endcase

  // enable where Q feeds the mux B input (inverted enable)
  always @(posedge clk) q_inv_en <= en ? q_inv_en : d;

  // flip-flop that already has an enable, plus another hold mux in front
  wire [3:0] nxt = en2 ? d : q_has_en;
  $dffe #(.CLK_POLARITY(1'b1), .EN_POLARITY(1'b1), .WIDTH(4)) has_en (.CLK(clk), .EN(en), .D(nxt), .Q(q_has_en));

  // two chained sync resets of opposite polarity
  always @(posedge clk)
    if (r1) q_chain <= 4'b0000;
    else if (!r2) q_chain <= 4'b0000;
    else q_chain <= d;

  // the same select used twice on the feedback path
  always @(posedge clk)
    if (en) begin
      if (en) q_sel_path <= d; else q_sel_path <= q_sel_path;
    end

  // always-active async load together with async reset / set-clear
  $aldff #(.CLK_POLARITY(1'b1), .ALOAD_POLARITY(1'b1), .WIDTH(4)) al_arst_core (.CLK(clk), .ALOAD(1'b1), .AD(ad), .D(d), .Q(q_al_arst));
  $dffsr #(.CLK_POLARITY(1'b1), .SET_POLARITY(1'b1), .CLR_POLARITY(1'b1), .WIDTH(4)) al_sr (.CLK(clk), .SET({r1, r1, 2'b00}), .CLR({2'b00, r2, r2}), .D(d), .Q(q_al_sr));

  // $sdffce with D == Q (sync reset gated by the enable)
  $sdffce #(.CLK_POLARITY(1'b1), .SRST_POLARITY(1'b1), .SRST_VALUE(4'b1001), .EN_POLARITY(1'b0), .WIDTH(4)) sdffce_dq (.CLK(clk), .SRST(r1), .EN(en), .D(q_sdffce_dq), .Q(q_sdffce_dq));

  // async-load flip-flop whose D holds its value: candidate for -sat
  (* init = 4'b0000 *) wire [3:0] q_aldff_sat;
  $aldff #(.CLK_POLARITY(1'b1), .ALOAD_POLARITY(1'b1), .WIDTH(4)) aldff_sat (.CLK(clk), .ALOAD(l), .AD(4'b0000), .D(q_aldff_sat & d), .Q(q_aldff_sat));
endmodule
