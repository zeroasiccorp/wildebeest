# Fixture for the zsimplemap tests: one combinational cell of each type
# zsimplemap maps, plus width edge cases (zero-width A or Y, Y wider
# than 1, 1-bit and odd-width reductions, signed extension, $sop with
# an empty product).  Each cell drives its own output port.
# Generated once with a script; edit by hand if needed.
module \top
  wire width 5 input 1 \a
  wire width 3 input 2 \b
  wire input 3 \c
  wire width 6 output 4 \not_signed
  wire width 4 output 5 \pos_unsigned
  wire width 3 output 6 \buf3
  wire width 5 output 7 \and_signed
  wire width 5 output 8 \or_unsigned
  wire width 5 output 9 \xor
  wire width 5 output 10 \xnor
  wire width 3 output 11 \bweqx
  wire width 1 output 12 \rand
  wire width 2 output 13 \ror_wide_y
  wire width 1 output 14 \rxor_odd
  wire width 1 output 15 \rxnor
  wire width 1 output 16 \rbool
  wire width 1 output 17 \rand_1bit
  wire width 1 output 18 \rand_empty_a
  wire width 1 output 19 \ror_empty_a
  wire width 1 output 20 \rxor_empty_a
  wire width 1 output 21 \rxnor_empty_a
  wire width 1 output 22 \rbool_empty_a
  wire width 2 output 23 \lnot_wide_y
  wire width 1 output 24 \lnot_empty_a
  wire width 1 output 25 \land
  wire width 3 output 26 \lor_wide_y
  wire width 1 output 27 \eq
  wire width 1 output 28 \ne
  wire width 1 output 29 \eqx
  wire width 2 output 30 \nex
  wire width 3 output 31 \mux
  wire width 3 output 32 \bwmux
  wire width 2 output 33 \bmux
  wire width 1 output 34 \lut3
  wire width 1 output 35 \sop
  wire width 1 output 36 \sop_no_empty
  wire width 3 output 37 \slice
  wire width 5 output 38 \concat
  cell $not \c_not_signed
    parameter \A_SIGNED 1
    parameter \A_WIDTH 5
    parameter \Y_WIDTH 6
    connect \A \a
    connect \Y \not_signed
  end
  cell $pos \c_pos_unsigned
    parameter \A_SIGNED 0
    parameter \A_WIDTH 3
    parameter \Y_WIDTH 4
    connect \A \b
    connect \Y \pos_unsigned
  end
  cell $buf \c_buf3
    parameter \WIDTH 3
    connect \A \b
    connect \Y \buf3
  end
  cell $and \c_and_signed
    parameter \A_SIGNED 1
    parameter \B_SIGNED 1
    parameter \A_WIDTH 5
    parameter \B_WIDTH 3
    parameter \Y_WIDTH 5
    connect \A \a
    connect \B \b
    connect \Y \and_signed
  end
  cell $or \c_or_unsigned
    parameter \A_SIGNED 0
    parameter \B_SIGNED 0
    parameter \A_WIDTH 5
    parameter \B_WIDTH 3
    parameter \Y_WIDTH 5
    connect \A \a
    connect \B \b
    connect \Y \or_unsigned
  end
  cell $xor \c_xor
    parameter \A_SIGNED 0
    parameter \B_SIGNED 0
    parameter \A_WIDTH 5
    parameter \B_WIDTH 3
    parameter \Y_WIDTH 5
    connect \A \a
    connect \B \b
    connect \Y \xor
  end
  cell $xnor \c_xnor
    parameter \A_SIGNED 0
    parameter \B_SIGNED 0
    parameter \A_WIDTH 5
    parameter \B_WIDTH 3
    parameter \Y_WIDTH 5
    connect \A \a
    connect \B \b
    connect \Y \xnor
  end
  cell $bweqx \c_bweqx
    parameter \WIDTH 3
    connect \A \b
    connect \B \a [2:0]
    connect \Y \bweqx
  end
  cell $reduce_and \c_rand
    parameter \A_SIGNED 0
    parameter \A_WIDTH 5
    parameter \Y_WIDTH 1
    connect \A \a
    connect \Y \rand
  end
  cell $reduce_or \c_ror_wide_y
    parameter \A_SIGNED 0
    parameter \A_WIDTH 5
    parameter \Y_WIDTH 2
    connect \A \a
    connect \Y \ror_wide_y
  end
  cell $reduce_xor \c_rxor_odd
    parameter \A_SIGNED 0
    parameter \A_WIDTH 3
    parameter \Y_WIDTH 1
    connect \A \b
    connect \Y \rxor_odd
  end
  cell $reduce_xnor \c_rxnor
    parameter \A_SIGNED 0
    parameter \A_WIDTH 5
    parameter \Y_WIDTH 1
    connect \A \a
    connect \Y \rxnor
  end
  cell $reduce_bool \c_rbool
    parameter \A_SIGNED 0
    parameter \A_WIDTH 3
    parameter \Y_WIDTH 1
    connect \A \b
    connect \Y \rbool
  end
  cell $reduce_and \c_rand_1bit
    parameter \A_SIGNED 0
    parameter \A_WIDTH 1
    parameter \Y_WIDTH 1
    connect \A \c
    connect \Y \rand_1bit
  end
  cell $reduce_and \c_rand_empty_a
    parameter \A_SIGNED 0
    parameter \A_WIDTH 0
    parameter \Y_WIDTH 1
    connect \A { }
    connect \Y \rand_empty_a
  end
  cell $reduce_or \c_ror_empty_a
    parameter \A_SIGNED 0
    parameter \A_WIDTH 0
    parameter \Y_WIDTH 1
    connect \A { }
    connect \Y \ror_empty_a
  end
  cell $reduce_xor \c_rxor_empty_a
    parameter \A_SIGNED 0
    parameter \A_WIDTH 0
    parameter \Y_WIDTH 1
    connect \A { }
    connect \Y \rxor_empty_a
  end
  cell $reduce_xnor \c_rxnor_empty_a
    parameter \A_SIGNED 0
    parameter \A_WIDTH 0
    parameter \Y_WIDTH 1
    connect \A { }
    connect \Y \rxnor_empty_a
  end
  cell $reduce_bool \c_rbool_empty_a
    parameter \A_SIGNED 0
    parameter \A_WIDTH 0
    parameter \Y_WIDTH 1
    connect \A { }
    connect \Y \rbool_empty_a
  end
  cell $reduce_or \c_ror_empty_y
    parameter \A_SIGNED 0
    parameter \A_WIDTH 5
    parameter \Y_WIDTH 0
    connect \A \a
    connect \Y { }
  end
  cell $logic_not \c_lnot_wide_y
    parameter \A_SIGNED 0
    parameter \A_WIDTH 5
    parameter \Y_WIDTH 2
    connect \A \a
    connect \Y \lnot_wide_y
  end
  cell $logic_not \c_lnot_empty_a
    parameter \A_SIGNED 0
    parameter \A_WIDTH 0
    parameter \Y_WIDTH 1
    connect \A { }
    connect \Y \lnot_empty_a
  end
  cell $logic_not \c_lnot_empty_y
    parameter \A_SIGNED 0
    parameter \A_WIDTH 5
    parameter \Y_WIDTH 0
    connect \A \a
    connect \Y { }
  end
  cell $logic_and \c_land
    parameter \A_SIGNED 0
    parameter \B_SIGNED 0
    parameter \A_WIDTH 5
    parameter \B_WIDTH 3
    parameter \Y_WIDTH 1
    connect \A \a
    connect \B \b
    connect \Y \land
  end
  cell $logic_or \c_lor_wide_y
    parameter \A_SIGNED 0
    parameter \B_SIGNED 0
    parameter \A_WIDTH 3
    parameter \B_WIDTH 1
    parameter \Y_WIDTH 3
    connect \A \b
    connect \B \c
    connect \Y \lor_wide_y
  end
  cell $logic_and \c_land_empty_y
    parameter \A_SIGNED 0
    parameter \B_SIGNED 0
    parameter \A_WIDTH 5
    parameter \B_WIDTH 3
    parameter \Y_WIDTH 0
    connect \A \a
    connect \B \b
    connect \Y { }
  end
  cell $eq \c_eq
    parameter \A_SIGNED 0
    parameter \B_SIGNED 0
    parameter \A_WIDTH 5
    parameter \B_WIDTH 3
    parameter \Y_WIDTH 1
    connect \A \a
    connect \B \b
    connect \Y \eq
  end
  cell $ne \c_ne
    parameter \A_SIGNED 0
    parameter \B_SIGNED 0
    parameter \A_WIDTH 5
    parameter \B_WIDTH 3
    parameter \Y_WIDTH 1
    connect \A \a
    connect \B \b
    connect \Y \ne
  end
  cell $eqx \c_eqx
    parameter \A_SIGNED 0
    parameter \B_SIGNED 0
    parameter \A_WIDTH 5
    parameter \B_WIDTH 3
    parameter \Y_WIDTH 1
    connect \A \a
    connect \B \b
    connect \Y \eqx
  end
  cell $nex \c_nex
    parameter \A_SIGNED 0
    parameter \B_SIGNED 0
    parameter \A_WIDTH 5
    parameter \B_WIDTH 3
    parameter \Y_WIDTH 2
    connect \A \a
    connect \B \b
    connect \Y \nex
  end
  cell $mux \c_mux
    parameter \WIDTH 3
    connect \A \b
    connect \B \a [2:0]
    connect \S \c
    connect \Y \mux
  end
  cell $bwmux \c_bwmux
    parameter \WIDTH 3
    connect \A \b
    connect \B \a [4:2]
    connect \S \a [2:0]
    connect \Y \bwmux
  end
  cell $bmux \c_bmux
    parameter \WIDTH 2
    parameter \S_WIDTH 2
    connect \A { \a \b }
    connect \S \b [1:0]
    connect \Y \bmux
  end
  cell $lut \c_lut3
    parameter \WIDTH 3
    parameter \LUT 8'10010110
    connect \A \b
    connect \Y \lut3
  end
  cell $sop \c_sop
    parameter \WIDTH 2
    parameter \DEPTH 2
    parameter \TABLE 8'00000110
    connect \A \b [1:0]
    connect \Y \sop
  end
  cell $sop \c_sop_no_empty
    parameter \WIDTH 2
    parameter \DEPTH 2
    parameter \TABLE 8'10010110
    connect \A \b [1:0]
    connect \Y \sop_no_empty
  end
  cell $slice \c_slice
    parameter \OFFSET 1
    parameter \A_WIDTH 5
    parameter \Y_WIDTH 3
    connect \A \a
    connect \Y \slice
  end
  cell $concat \c_concat
    parameter \A_WIDTH 2
    parameter \B_WIDTH 3
    connect \A \a [1:0]
    connect \B \b
    connect \Y \concat
  end
end
