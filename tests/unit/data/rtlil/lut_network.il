# LUT network used by the max_height / report_stat tests.
#
# Critical path a -> l1 -> l2 -> l3 -> y is 3 LUTs high.  l3's third input is
# the undriven wire k: each test ties it with 'connect -set' (to a constant or
# to a signal).  l4 ($lut) and l5 (LUT2 model) are off the critical path.
# s1 is an SB_LUT4 with no module definition, so max_height cannot find its
# output port.  Names use the characters the .dot writer has to legalize.
module \top
  wire input 1 \a
  wire input 2 \b
  wire input 3 \c
  wire input 4 \d
  wire output 5 \y
  wire output 6 \z
  wire output 7 \w
  wire output 8 \v
  wire \n1
  wire \n2
  wire \k
  cell $lut \l1$x.y:z[0]
    parameter \WIDTH 2
    parameter \LUT 4'1000
    connect \A { \a \b }
    connect \Y \n1
  end
  cell $lut \l2\q
    parameter \WIDTH 2
    parameter \LUT 4'0110
    connect \A { \n1 \c }
    connect \Y \n2
  end
  cell $lut \l3
    parameter \WIDTH 3
    parameter \LUT 8'10010110
    connect \A { \n2 \n1 \k }
    connect \Y \y
  end
  cell $lut \l4
    parameter \WIDTH 2
    parameter \LUT 4'1110
    connect \A { \c \d }
    connect \Y \z
  end
  cell \LUT2 \l5
    parameter \INIT_VALUE 4'1000
    connect \I0 \a
    connect \I1 \d
    connect \O \w
  end
  cell \SB_LUT4 \s1
    connect \I0 \a
    connect \I1 \b
    connect \I2 \c
    connect \I3 \d
    connect \O \v
  end
end
