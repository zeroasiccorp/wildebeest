// zopt_dff fixture: constant async-load, async/sync reset, enable and clock
// inputs.  One cell per case (instance name = case), output Q_<case>.
// Read with -icells.
module top (input CLK, EN, R, L, S, C, input [3:0] D, AD, output [3:0] Q_al_never0, Q_al_neverx, Q_al_always, Q_al_always_en, Q_al_const_ad, Q_lat_arst_on, Q_lat_on, Q_ar_never, Q_ar_always, Q_sr_never, Q_sr_always_e, Q_sr_always_ce, Q_en_always, Q_en_never_sdffe, Q_clk_const_arst, Q_f_ar_never, Q_f_ar_always, Q_f_sr_always, Q_f_en_always, Q_f_al_always, Q_f_lat_on);
  $aldff #(.CLK_POLARITY(1'b1), .ALOAD_POLARITY(1'b1), .WIDTH(4)) al_never0 (.CLK(CLK), .ALOAD(1'b0), .AD(AD), .D(D), .Q(Q_al_never0));
  $aldff #(.CLK_POLARITY(1'b1), .ALOAD_POLARITY(1'b0), .WIDTH(4)) al_neverx (.CLK(CLK), .ALOAD(1'bx), .AD(AD), .D(D), .Q(Q_al_neverx));
  $aldff #(.CLK_POLARITY(1'b1), .ALOAD_POLARITY(1'b1), .WIDTH(4)) al_always (.CLK(CLK), .ALOAD(1'b1), .AD(AD), .D(D), .Q(Q_al_always));
  $aldffe #(.CLK_POLARITY(1'b1), .ALOAD_POLARITY(1'b0), .EN_POLARITY(1'b1), .WIDTH(4)) al_always_en (.CLK(CLK), .EN(EN), .ALOAD(1'b0), .AD(AD), .D(D), .Q(Q_al_always_en));
  $aldff #(.CLK_POLARITY(1'b1), .ALOAD_POLARITY(1'b1), .WIDTH(4)) al_const_ad (.CLK(CLK), .ALOAD(L), .AD(4'b1010), .D(D), .Q(Q_al_const_ad));
  $adlatch #(.EN_POLARITY(1'b1), .ARST_POLARITY(1'b1), .ARST_VALUE(4'b0110), .WIDTH(4)) lat_arst_on (.EN(1'b1), .ARST(R), .D(D), .Q(Q_lat_arst_on));
  $dlatch #(.EN_POLARITY(1'b0), .WIDTH(4)) lat_on (.EN(1'b0), .D(D), .Q(Q_lat_on));
  $adff #(.CLK_POLARITY(1'b1), .ARST_POLARITY(1'b1), .ARST_VALUE(4'b0101), .WIDTH(4)) ar_never (.CLK(CLK), .ARST(1'b0), .D(D), .Q(Q_ar_never));
  $adff #(.CLK_POLARITY(1'b1), .ARST_POLARITY(1'b0), .ARST_VALUE(4'b0101), .WIDTH(4)) ar_always (.CLK(CLK), .ARST(1'b0), .D(D), .Q(Q_ar_always));
  $sdff #(.CLK_POLARITY(1'b1), .SRST_POLARITY(1'b1), .SRST_VALUE(4'b0011), .WIDTH(4)) sr_never (.CLK(CLK), .SRST(1'b0), .D(D), .Q(Q_sr_never));
  $sdffe #(.CLK_POLARITY(1'b1), .SRST_POLARITY(1'b1), .SRST_VALUE(4'b0011), .EN_POLARITY(1'b1), .WIDTH(4)) sr_always_e (.CLK(CLK), .SRST(1'b1), .EN(EN), .D(D), .Q(Q_sr_always_e));
  $sdffce #(.CLK_POLARITY(1'b1), .SRST_POLARITY(1'b0), .SRST_VALUE(4'b1100), .EN_POLARITY(1'b1), .WIDTH(4)) sr_always_ce (.CLK(CLK), .SRST(1'b0), .EN(EN), .D(D), .Q(Q_sr_always_ce));
  $dffe #(.CLK_POLARITY(1'b1), .EN_POLARITY(1'b0), .WIDTH(4)) en_always (.CLK(CLK), .EN(1'b0), .D(D), .Q(Q_en_always));
  $sdffe #(.CLK_POLARITY(1'b1), .SRST_POLARITY(1'b1), .SRST_VALUE(4'b1111), .EN_POLARITY(1'b1), .WIDTH(4)) en_never_sdffe (.CLK(CLK), .SRST(R), .EN(1'b0), .D(D), .Q(Q_en_never_sdffe));
  $adff #(.CLK_POLARITY(1'b1), .ARST_POLARITY(1'b1), .ARST_VALUE(4'b1001), .WIDTH(4)) clk_const_arst (.CLK(1'b1), .ARST(R), .D(D), .Q(Q_clk_const_arst));
  $_DFF_PP0_ f_ar_never (.C(CLK), .R(1'b0), .D(D[0]), .Q(Q_f_ar_never[0])); assign Q_f_ar_never[3:1] = 3'b000;
  $_DFF_PP1_ f_ar_always (.C(CLK), .R(1'b1), .D(D[0]), .Q(Q_f_ar_always[0])); assign Q_f_ar_always[3:1] = 3'b000;
  $_SDFF_PN0_ f_sr_always (.C(CLK), .R(1'b0), .D(D[0]), .Q(Q_f_sr_always[0])); assign Q_f_sr_always[3:1] = 3'b000;
  $_DFFE_PP_ f_en_always (.C(CLK), .E(1'b1), .D(D[0]), .Q(Q_f_en_always[0])); assign Q_f_en_always[3:1] = 3'b000;
  $_ALDFF_PN_ f_al_always (.C(CLK), .L(1'b0), .AD(AD[0]), .D(D[0]), .Q(Q_f_al_always[0])); assign Q_f_al_always[3:1] = 3'b000;
  $_DLATCH_N_ f_lat_on (.E(1'b0), .D(D[0]), .Q(Q_f_lat_on[0])); assign Q_f_lat_on[3:1] = 3'b000;
endmodule
