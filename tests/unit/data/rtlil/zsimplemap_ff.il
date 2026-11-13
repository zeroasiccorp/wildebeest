# Fixture for the zsimplemap flip-flop test: one 2-bit cell of every
# storage type zsimplemap maps, a zero-width $dff and a $tribuf.
# Generated once with a script; edit by hand if needed.
module \top
  wire input 1 \clk
  wire input 2 \en
  wire input 3 \rst
  wire width 2 input 4 \s
  wire width 2 input 5 \r
  wire width 2 input 6 \ad
  wire width 2 input 7 \d
  wire width 2 output 8 \q_sr
  wire width 2 output 9 \q_ff
  wire width 2 output 10 \q_dff
  wire width 2 output 11 \q_dffe
  wire width 2 output 12 \q_dffsr
  wire width 2 output 13 \q_dffsre
  wire width 2 output 14 \q_adff
  wire width 2 output 15 \q_sdff
  wire width 2 output 16 \q_adffe
  wire width 2 output 17 \q_sdffe
  wire width 2 output 18 \q_sdffce
  wire width 2 output 19 \q_aldff
  wire width 2 output 20 \q_aldffe
  wire width 2 output 21 \q_dlatch
  wire width 2 output 22 \q_adlatch
  wire width 2 output 23 \q_dlatchsr
  wire width 2 output 24 \y_tribuf
  cell $sr \c_sr
    parameter \WIDTH 2
    parameter \SET_POLARITY 1
    parameter \CLR_POLARITY 0
    connect \SET \s
    connect \CLR \r
    connect \Q \q_sr
  end
  cell $ff \c_ff
    parameter \WIDTH 2
    connect \D \d
    connect \Q \q_ff
  end
  cell $dff \c_dff
    parameter \WIDTH 2
    parameter \CLK_POLARITY 1
    connect \CLK \clk
    connect \D \d
    connect \Q \q_dff
  end
  cell $dffe \c_dffe
    parameter \WIDTH 2
    parameter \CLK_POLARITY 1
    parameter \EN_POLARITY 0
    connect \CLK \clk
    connect \EN \en
    connect \D \d
    connect \Q \q_dffe
  end
  cell $dffsr \c_dffsr
    parameter \WIDTH 2
    parameter \CLK_POLARITY 0
    parameter \SET_POLARITY 1
    parameter \CLR_POLARITY 1
    connect \CLK \clk
    connect \SET \s
    connect \CLR \r
    connect \D \d
    connect \Q \q_dffsr
  end
  cell $dffsre \c_dffsre
    parameter \WIDTH 2
    parameter \CLK_POLARITY 1
    parameter \SET_POLARITY 0
    parameter \CLR_POLARITY 1
    parameter \EN_POLARITY 1
    connect \CLK \clk
    connect \SET \s
    connect \CLR \r
    connect \EN \en
    connect \D \d
    connect \Q \q_dffsre
  end
  cell $adff \c_adff
    parameter \WIDTH 2
    parameter \CLK_POLARITY 1
    parameter \ARST_POLARITY 0
    parameter \ARST_VALUE 2'10
    connect \CLK \clk
    connect \ARST \rst
    connect \D \d
    connect \Q \q_adff
  end
  cell $sdff \c_sdff
    parameter \WIDTH 2
    parameter \CLK_POLARITY 1
    parameter \SRST_POLARITY 1
    parameter \SRST_VALUE 2'01
    connect \CLK \clk
    connect \SRST \rst
    connect \D \d
    connect \Q \q_sdff
  end
  cell $adffe \c_adffe
    parameter \WIDTH 2
    parameter \CLK_POLARITY 1
    parameter \ARST_POLARITY 1
    parameter \ARST_VALUE 2'11
    parameter \EN_POLARITY 1
    connect \CLK \clk
    connect \ARST \rst
    connect \EN \en
    connect \D \d
    connect \Q \q_adffe
  end
  cell $sdffe \c_sdffe
    parameter \WIDTH 2
    parameter \CLK_POLARITY 1
    parameter \SRST_POLARITY 0
    parameter \SRST_VALUE 2'00
    parameter \EN_POLARITY 1
    connect \CLK \clk
    connect \SRST \rst
    connect \EN \en
    connect \D \d
    connect \Q \q_sdffe
  end
  cell $sdffce \c_sdffce
    parameter \WIDTH 2
    parameter \CLK_POLARITY 1
    parameter \SRST_POLARITY 1
    parameter \SRST_VALUE 2'10
    parameter \EN_POLARITY 0
    connect \CLK \clk
    connect \SRST \rst
    connect \EN \en
    connect \D \d
    connect \Q \q_sdffce
  end
  cell $aldff \c_aldff
    parameter \WIDTH 2
    parameter \CLK_POLARITY 1
    parameter \ALOAD_POLARITY 1
    connect \CLK \clk
    connect \ALOAD \rst
    connect \AD \ad
    connect \D \d
    connect \Q \q_aldff
  end
  cell $aldffe \c_aldffe
    parameter \WIDTH 2
    parameter \CLK_POLARITY 0
    parameter \ALOAD_POLARITY 0
    parameter \EN_POLARITY 1
    connect \CLK \clk
    connect \ALOAD \rst
    connect \AD \ad
    connect \EN \en
    connect \D \d
    connect \Q \q_aldffe
  end
  cell $dlatch \c_dlatch
    parameter \WIDTH 2
    parameter \EN_POLARITY 1
    connect \EN \en
    connect \D \d
    connect \Q \q_dlatch
  end
  cell $adlatch \c_adlatch
    parameter \WIDTH 2
    parameter \EN_POLARITY 1
    parameter \ARST_POLARITY 1
    parameter \ARST_VALUE 2'01
    connect \EN \en
    connect \ARST \rst
    connect \D \d
    connect \Q \q_adlatch
  end
  cell $dlatchsr \c_dlatchsr
    parameter \WIDTH 2
    parameter \EN_POLARITY 0
    parameter \SET_POLARITY 1
    parameter \CLR_POLARITY 0
    connect \EN \en
    connect \SET \s
    connect \CLR \r
    connect \D \d
    connect \Q \q_dlatchsr
  end
  cell $dff \c_dff_w0
    parameter \WIDTH 0
    parameter \CLK_POLARITY 1
    connect \CLK \clk
    connect \D { }
    connect \Q { }
  end
  cell $tribuf \c_tribuf
    parameter \WIDTH 2
    connect \A \d
    connect \EN \en
    connect \Y \y_tribuf
  end
end
