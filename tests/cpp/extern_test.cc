// C++ unit tests that need no '#include' of a source file: the zsimplemap
// mappers and ZQuickConeSat are external functions / classes, and the crash
// bugs below are reached through ordinary pass calls.
#include "zqcsat.h"
#include "zsimplemap.h"

#include "kernel/modtools.h"
#include "wb_test.h"

USING_YOSYS_NAMESPACE

using wbtest::load;

// ------------------------------------------------------------- zsimplemap

#ifndef NDEBUG
// The mappers are only called for the cell types they are registered for;
// called with another type that has the same ports they hit a log_assert.
WB_TEST(zsimplemap_mapper_type_asserts) {
  RTLIL::Design *design = yosys_get_design();
  RTLIL::Module *top = load(design,
      "module top(input [1:0] a, b, output [1:0] y, output [2:0] s);\n"
      "  assign y = ~a;\n"
      "  assign s = a + b;\n"
      "endmodule");
  Cell *not_cell = nullptr, *add_cell = nullptr;
  for (auto cell : top->cells()) {
    if (cell->type == ID($not))
      not_cell = cell;
    if (cell->type == ID($add))
      add_cell = cell;
  }
  WB_CHECK(not_cell != nullptr && add_cell != nullptr);
  WB_CHECK_LOG_ERROR(zsimplemap_bitop(top, add_cell));
  WB_CHECK_LOG_ERROR(zsimplemap_reduce(top, not_cell));
  WB_CHECK_LOG_ERROR(zsimplemap_logbin(top, add_cell));
}
#endif

// ----------------------------------------------------------------- zqcsat

// importSig() and the max_cell_outs / max_cell_count limits are not used by
// zopt_dff; exercise them directly.
WB_TEST(zqcsat_import_and_limits) {
  RTLIL::Design *design = yosys_get_design();
  RTLIL::Module *top = load(design,
      "module top(input [1:0] a, b, input c, d, e, output [1:0] s, output y);\n"
      "  assign s = a + b;\n"
      "  assign y = (((s[0] & c) | s[1]) ^ d) & e;\n"
      "endmodule");
  ModWalker modwalker(design, top);

  ZQuickConeSat unlimited(modwalker);
  unlimited.importSig(top->wire(ID(y)));
  unlimited.prepare();
  WB_CHECK(unlimited.nbImportedCells() >= 3);

  // The 2-output $add is skipped when cells may have at most one output.
  ZQuickConeSat few_outputs(modwalker);
  few_outputs.max_cell_outs = 1;
  few_outputs.importSig(top->wire(ID(y)));
  few_outputs.prepare();
  WB_CHECK(few_outputs.nbImportedCells() < unlimited.nbImportedCells());

  // The limit is checked after each round of imports; with a cone several
// cells deep, import stops early.
  ZQuickConeSat few_cells(modwalker);
  few_cells.max_cell_count = 1;
  few_cells.importSig(top->wire(ID(y)));
  few_cells.prepare();
  WB_CHECK(few_cells.nbImportedCells() < unlimited.nbImportedCells());

  // Complexity classes, on parameter-less probe cells that are removed again
  // (Yosys checks every cell's parameters after the pass).
  auto complexity = [&](RTLIL::IdString type) {
    RTLIL::Cell *probe = top->addCell(NEW_ID, type);
    int c = ZQuickConeSat::cell_complexity(probe);
    top->remove(probe);
    return c;
  };
  WB_CHECK_EQ(complexity(ID($concat)), 0);
  WB_CHECK_EQ(complexity(ID($sub)), 2);
  WB_CHECK_EQ(complexity(ID($sshr)), 3);
  WB_CHECK_EQ(complexity(ID($div)), 4);
  WB_CHECK_EQ(complexity(ID($dff)), 5);
}

// ------------------------------------------- crash bugs (KNOWN_ISSUES.md)
//
// These throw std::out_of_range out of the pass, which aborts Yosys when it
// happens from a script.  The tests pin today's behaviour; when a bug is
// fixed, change the check to the new expected behaviour.

// KNOWN-ISSUE ML1: max_level with a selection that contains cells but only
// some of the wires they drive.
WB_TEST(max_level_mixed_selection_throws) {
  RTLIL::Design *design = yosys_get_design();
  load(design, "module top(input a, b, c, output y); wire t = a & b; assign y = t | c; endmodule");
  WB_CHECK_THROWS(std::out_of_range, Pass::call(design, "max_level t:$and w:a"));
}

// KNOWN-ISSUE MH2: max_height on a partial selection with LUTs but no wires.
// (max_height ignores selection arguments, KNOWN-ISSUE MH1, so the
// selection is made with 'select' first.)
WB_TEST(max_height_partial_selection_throws) {
  RTLIL::Design *design = yosys_get_design();
  load(design, "module top(input [3:0] a, output y); assign y = ^a; endmodule");
  Pass::call(design, "synth -lut 4");
  Pass::call(design, "select t:$lut");
  WB_CHECK_THROWS(std::out_of_range, Pass::call(design, "max_height"));
}

// KNOWN-ISSUE MH5: max_height -dot with a constant input on a critical-path
// LUT.
WB_TEST(max_height_dot_constant_input_throws) {
  RTLIL::Design *design = yosys_get_design();
  Pass::call(design, "design -reset");
  RTLIL::Module *top = design->addModule(ID(top));
  RTLIL::Wire *a = top->addWire(ID(a)), *b = top->addWire(ID(b));
  RTLIL::Wire *n = top->addWire(ID(n)), *y = top->addWire(ID(y));
  a->port_input = b->port_input = true;
  y->port_output = true;
  top->fixup_ports();
  top->addLut(ID(l1), {a, b}, n, RTLIL::Const(8, 4));
  top->addLut(ID(l2), {n, RTLIL::SigSpec(RTLIL::State::S1)}, y, RTLIL::Const(6, 4));
  WB_CHECK_THROWS(std::out_of_range, Pass::call(design, "max_height -dot"));
}
