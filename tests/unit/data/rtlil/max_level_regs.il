# Fixture for max_level-basic: the result of proc + opt_clean on
#   module top (input clk, a, b, c, d, output y, output reg q, r);
#     wire t = (a & b) | c;  assign y = t ^ d;
#     always @(posedge clk) q <= y & a;
#     always @(posedge clk) r <= q ^ r;
# kept as RTLIL so every Yosys version sees the same netlist (older Yosys
# versions build a different one from the Verilog).
autoidx 10
module \top
  wire input 1 \clk
  wire input 2 \a
  wire input 3 \b
  wire input 4 \c
  wire input 5 \d
  wire output 6 \y
  wire output 7 \q
  wire output 8 \r
  wire \t
  wire $and$<inlined>:2$1_Y
  wire $0\q[0:0]
  wire $0\r[0:0]
  cell $and $and$<inlined>:2$1
    parameter \A_SIGNED 0
    parameter \B_SIGNED 0
    parameter \A_WIDTH 1
    parameter \B_WIDTH 1
    parameter \Y_WIDTH 1
    connect \A \a
    connect \B \b
    connect \Y $and$<inlined>:2$1_Y
  end
  cell $or $or$<inlined>:2$2
    parameter \A_SIGNED 0
    parameter \B_SIGNED 0
    parameter \A_WIDTH 1
    parameter \B_WIDTH 1
    parameter \Y_WIDTH 1
    connect \A $and$<inlined>:2$1_Y
    connect \B \c
    connect \Y \t
  end
  cell $xor $xor$<inlined>:3$3
    parameter \A_SIGNED 0
    parameter \B_SIGNED 0
    parameter \A_WIDTH 1
    parameter \B_WIDTH 1
    parameter \Y_WIDTH 1
    connect \A \t
    connect \B \d
    connect \Y \y
  end
  cell $and $and$<inlined>:4$5
    parameter \A_SIGNED 0
    parameter \B_SIGNED 0
    parameter \A_WIDTH 1
    parameter \B_WIDTH 1
    parameter \Y_WIDTH 1
    connect \A \y
    connect \B \a
    connect \Y $0\q[0:0]
  end
  cell $xor $xor$<inlined>:5$7
    parameter \A_SIGNED 0
    parameter \B_SIGNED 0
    parameter \A_WIDTH 1
    parameter \B_WIDTH 1
    parameter \Y_WIDTH 1
    connect \A \q
    connect \B \r
    connect \Y $0\r[0:0]
  end
  cell $dff $procdff$8
    parameter \WIDTH 1
    parameter \CLK_POLARITY 1'1
    connect \D $0\r[0:0]
    connect \Q \r
    connect \CLK \clk
  end
  cell $dff $procdff$9
    parameter \WIDTH 1
    parameter \CLK_POLARITY 1'1
    connect \D $0\q[0:0]
    connect \Q \q
    connect \CLK \clk
  end
end
