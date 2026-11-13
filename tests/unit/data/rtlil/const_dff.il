# Fixture for the zopt_const_dff tests: one flip-flop per case, each driving
# its own output.  Init values are set with the 'init' attribute on Q.
#
#   f_d0_i0     $_DFF_P_     D=0 init 0          -> replaced by 0
#   f_d1_i1     $_DFF_P_     D=1 init 1          -> replaced by 1
#   f_d1_i0     $_DFF_P_     D=1 init 0          -> conflict warning, kept
#   f_d0_i1     $_DFF_P_     D=0 init 1          -> conflict warning, kept
#   f_d0_nx     $_DFF_P_     D=0 no init         -> (OC2) conflict warning, kept
#   f_d1_nx     $_DFF_P_     D=1 no init         -> (OC2) "replaced", Q tied to x
#   f_d_sig     $_DFF_P_     D=a                 -> not constant, untouched
#   c_dff       $dff         D=1 init 1          -> coarse cell, skipped
#   f_ar_off    $_DFF_PN0_   D=1 init 1, R=1     -> reset never active, replaced
#   f_ar_d1     $_DFF_PN0_   D=1 init 1, R=r     -> reset can fire, kept
#   f_ar1_d0    $_DFF_PN1_   D=0 init 0, R=r     -> (OC3) replaced by 0 although
#                                                   the reset sets Q to 1
#   f_sr_off    $_SDFF_PP0_  D=1 init 1, R=0     -> reset never active, replaced
#   f_sr_d1     $_SDFF_PP0_  D=1 init 1, R=r     -> reset can fire, kept
#   f_dffsr     $_DFFSR_PPP_ D=1 init 1          -> NOTE, kept
#   f_ce        $_DFFE_PP_   D=0 init 0, E=e     -> enable ignored, replaced
module \top
  wire input 1 \clk
  wire input 2 \a
  wire input 3 \r
  wire input 4 \e
  attribute \init 1'0
  wire output 5 \q_d0_i0
  attribute \init 1'1
  wire output 6 \q_d1_i1
  attribute \init 1'0
  wire output 7 \q_d1_i0
  attribute \init 1'1
  wire output 8 \q_d0_i1
  wire output 9 \q_d0_nx
  wire output 10 \q_d1_nx
  wire output 11 \q_d_sig
  attribute \init 1'1
  wire output 12 \q_c_dff
  attribute \init 1'1
  wire output 13 \q_ar_off
  attribute \init 1'1
  wire output 14 \q_ar_d1
  attribute \init 1'0
  wire output 15 \q_ar1_d0
  attribute \init 1'1
  wire output 16 \q_sr_off
  attribute \init 1'1
  wire output 17 \q_sr_d1
  attribute \init 1'1
  wire output 18 \q_dffsr
  attribute \init 1'0
  wire output 19 \q_ce
  cell $_DFF_P_ \f_d0_i0
    connect \C \clk
    connect \D 1'0
    connect \Q \q_d0_i0
  end
  cell $_DFF_P_ \f_d1_i1
    connect \C \clk
    connect \D 1'1
    connect \Q \q_d1_i1
  end
  cell $_DFF_P_ \f_d1_i0
    connect \C \clk
    connect \D 1'1
    connect \Q \q_d1_i0
  end
  cell $_DFF_P_ \f_d0_i1
    connect \C \clk
    connect \D 1'0
    connect \Q \q_d0_i1
  end
  cell $_DFF_P_ \f_d0_nx
    connect \C \clk
    connect \D 1'0
    connect \Q \q_d0_nx
  end
  cell $_DFF_P_ \f_d1_nx
    connect \C \clk
    connect \D 1'1
    connect \Q \q_d1_nx
  end
  cell $_DFF_P_ \f_d_sig
    connect \C \clk
    connect \D \a
    connect \Q \q_d_sig
  end
  cell $dff \c_dff
    parameter \WIDTH 1
    parameter \CLK_POLARITY 1
    connect \CLK \clk
    connect \D 1'1
    connect \Q \q_c_dff
  end
  cell $_DFF_PN0_ \f_ar_off
    connect \C \clk
    connect \R 1'1
    connect \D 1'1
    connect \Q \q_ar_off
  end
  cell $_DFF_PN0_ \f_ar_d1
    connect \C \clk
    connect \R \r
    connect \D 1'1
    connect \Q \q_ar_d1
  end
  cell $_DFF_PN1_ \f_ar1_d0
    connect \C \clk
    connect \R \r
    connect \D 1'0
    connect \Q \q_ar1_d0
  end
  cell $_SDFF_PP0_ \f_sr_off
    connect \C \clk
    connect \R 1'0
    connect \D 1'1
    connect \Q \q_sr_off
  end
  cell $_SDFF_PP0_ \f_sr_d1
    connect \C \clk
    connect \R \r
    connect \D 1'1
    connect \Q \q_sr_d1
  end
  cell $_DFFSR_PPP_ \f_dffsr
    connect \C \clk
    connect \S \r
    connect \R \e
    connect \D 1'1
    connect \Q \q_dffsr
  end
  cell $_DFFE_PP_ \f_ce
    connect \C \clk
    connect \E \e
    connect \D 1'0
    connect \Q \q_ce
  end
end
