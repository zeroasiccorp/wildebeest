# Fixture for the obs_clean tests.
#
# Observable (in the fan-in of an output or of a (* keep *) wire):
#   t1 = a & b, y = t1 | c, kw = ~a (kw has keep), y2 = t1 (assign),
#   z = m[0] where m = {a, b} (only one bit of the assign is used),
#   z2 = p where {p, q} = d (assign with a two-chunk left side).
# Not observable:
#   u1 = a & b, u2 = a & c, u3 = u1 | u2 (three dead cells, two of type $and),
#   dangling = u1 (unused assign), unused_w (wire with no use at all).
# kx = a ^ b is not observable either, but the cell has keep.
module \top
  wire input 1 \a
  wire input 2 \b
  wire input 3 \c
  wire width 2 input 4 \d
  wire output 5 \y
  wire output 6 \y2
  wire output 7 \z
  wire output 8 \z2
  wire \t1
  wire \u1
  wire \u2
  wire \u3
  wire \kx
  attribute \keep 1
  wire \kw
  wire \dangling
  wire \unused_w
  wire width 2 \m
  wire \p
  wire \q
  cell $and \g_t1
    parameter \A_SIGNED 0
    parameter \B_SIGNED 0
    parameter \A_WIDTH 1
    parameter \B_WIDTH 1
    parameter \Y_WIDTH 1
    connect \A \a
    connect \B \b
    connect \Y \t1
  end
  cell $or \g_y
    parameter \A_SIGNED 0
    parameter \B_SIGNED 0
    parameter \A_WIDTH 1
    parameter \B_WIDTH 1
    parameter \Y_WIDTH 1
    connect \A \t1
    connect \B \c
    connect \Y \y
  end
  cell $not \g_kw
    parameter \A_SIGNED 0
    parameter \A_WIDTH 1
    parameter \Y_WIDTH 1
    connect \A \a
    connect \Y \kw
  end
  cell $and \dead_u1
    parameter \A_SIGNED 0
    parameter \B_SIGNED 0
    parameter \A_WIDTH 1
    parameter \B_WIDTH 1
    parameter \Y_WIDTH 1
    connect \A \a
    connect \B \b
    connect \Y \u1
  end
  cell $and \dead_u2
    parameter \A_SIGNED 0
    parameter \B_SIGNED 0
    parameter \A_WIDTH 1
    parameter \B_WIDTH 1
    parameter \Y_WIDTH 1
    connect \A \a
    connect \B \c
    connect \Y \u2
  end
  cell $or \dead_u3
    parameter \A_SIGNED 0
    parameter \B_SIGNED 0
    parameter \A_WIDTH 1
    parameter \B_WIDTH 1
    parameter \Y_WIDTH 1
    connect \A \u1
    connect \B \u2
    connect \Y \u3
  end
  attribute \keep 1
  cell $xor \kept_x
    parameter \A_SIGNED 0
    parameter \B_SIGNED 0
    parameter \A_WIDTH 1
    parameter \B_WIDTH 1
    parameter \Y_WIDTH 1
    connect \A \a
    connect \B \b
    connect \Y \kx
  end
  connect \y2 \t1
  connect \dangling \u1
  connect \m { \a \b }
  connect \z \m [0]
  connect { \p \q } \d
  connect \z2 \p
end
