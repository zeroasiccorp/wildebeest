// C++ unit tests for src/synth_fpga.cc: code the script tests cannot reach.
//
// The source is included so the tests can use the SynthFpgaPass object and
// its members, which live in an anonymous namespace.
#include "../../src/synth_fpga.cc"

#include "wb_test.h"

USING_YOSYS_NAMESPACE

using wbtest::load;

// --------------------------------------------------------------- spike cases

WB_TEST(synth_fpga_null_design_guards) {
  wbtest::NullDesign no_design;
  wbtest::LogCapture log;
  SynthFpgaPass.analyze_undriven_nets(nullptr, true);
  SynthFpgaPass.check_DLatch();
  SynthFpgaPass.check_illegal_cells();
  SynthFpgaPass.processDffInitValues(0);
  WB_CHECK_EQ(SynthFpgaPass.getNumberOfLuts(), -1);
  WB_CHECK_EQ(SynthFpgaPass.getNumberOfDffs(), -1);
  SynthFpgaPass.dump_csv_file("unused.csv", 0);
  SynthFpgaPass.abc_synthesize();
  SynthFpgaPass.script();
  WB_CHECK(log.contains("Design seems empty"));
}

WB_TEST(synth_fpga_xor_tree_needs_two_leaves) {
  RTLIL::Design *design = yosys_get_design();
  RTLIL::Module *top = load(design, "module top(input a, output y); assign y = a; endmodule");
  dict<RTLIL::SigSpec, Cell *> y2xor;
  RTLIL::SigSpec y = top->wire(ID(y)), new_y;
  vector<RTLIL::SigSpec> leaves = {RTLIL::SigSpec(top->wire(ID(a)))};
  WB_CHECK_LOG_ERROR(
      SynthFpgaPass.build_binary_xor_tree_rec(top, y, y2xor, new_y, leaves));
}

WB_TEST(synth_fpga_show_sig) {
  RTLIL::Design *design = yosys_get_design();
  RTLIL::Module *top = load(design,
      "module top(input [7:0] a, input [0:3] b, output y); assign y = a[0]; endmodule");
  RTLIL::SigSpec a = top->wire(ID(a)), b = top->wire(ID(b));
  wbtest::LogCapture log;
  SynthFpgaPass.show_sig(RTLIL::SigSpec());               // empty
  SynthFpgaPass.show_sig(a);                              // whole wire
  SynthFpgaPass.show_sig(a.extract(3, 1));                // one bit
  SynthFpgaPass.show_sig(a.extract(2, 4));                // slice
  SynthFpgaPass.show_sig(b.extract(1, 1));                // one bit, upto
  SynthFpgaPass.show_sig(b.extract(0, 2));                // slice, upto
  SynthFpgaPass.show_sig(RTLIL::SigSpec(RTLIL::Const(5, 3)));   // constant
#if YOSYS_MAJOR == 0 && YOSYS_MINOR < 70
  // KNOWN-ISSUE SF19: on Yosys 0.70+ SigSpec::chunks() returns a temporary
  // view, so the multi-chunk loop compares iterators of two different
  // temporaries and crashes.  show_sig is never called by the plugin.
  RTLIL::SigSpec mixed = a.extract(0, 2);
  mixed.append(RTLIL::Const(1, 1));
  SynthFpgaPass.show_sig(mixed);                          // several chunks
  WB_CHECK(log.contains("{ "));
#endif
  const std::string text = log.text();
  WB_CHECK(text.find("{0{1'b0}}") != std::string::npos);
  WB_CHECK(text.find("a[3]") != std::string::npos);
  WB_CHECK(text.find("a[5:2]") != std::string::npos);
  WB_CHECK(text.find("b[2]") != std::string::npos);    // offset 1 of [0:3]
  WB_CHECK(text.find("b[2:3]") != std::string::npos);  // offsets 0-1 of [0:3]
  WB_CHECK(text.find("CONSTANT") != std::string::npos);
}

// --------------------------------------------------------- helpers and lists

WB_TEST(synth_fpga_sort_helpers) {
  SynthFpgaPass::xor_head low, high;
  low.height = 1;
  high.height = 3;
  WB_CHECK(SynthFpgaPass::cmpHeight(&high, &low));
  WB_CHECK(!SynthFpgaPass::cmpHeight(&low, &high));
  WB_CHECK(SynthFpgaPass::cmpInvHeight(&low, &high));
  WB_CHECK(!SynthFpgaPass::cmpInvHeight(&high, &low));
  SynthFpgaPass::leaf_info first, second;
  first.Id = 1;
  second.Id = 2;
  WB_CHECK(SynthFpgaPass::cmpId(&first, &second));
  WB_CHECK(!SynthFpgaPass::cmpId(&second, &first));
  WB_CHECK_EQ(SynthFpgaPass::id(ID(foo)), std::string("\\foo"));
}

// Without -partname z1000 or a platypus part (impossible from the command
// line, since check_options() rejects other names) a generic legal flop list
// is used, with or without sync-reset flops.
WB_TEST(synth_fpga_generic_legal_flops) {
  auto has = [](const std::string &type) {
    auto &l = SynthFpgaPass.ys_legal_flops;
    return std::find(l.begin(), l.end(), type) != l.end();
  };
  SynthFpgaPass.clear_flags();
  SynthFpgaPass.ys_legal_flops.clear();
  SynthFpgaPass.part_name = "generic";
  SynthFpgaPass.setup_options();
  WB_CHECK(has("$_DFF_PN?_"));
  WB_CHECK(has("$_DFFSRE_PNNP_"));
  WB_CHECK(has("$_SDFF_P??_"));

  SynthFpgaPass.clear_flags();
  SynthFpgaPass.ys_legal_flops.clear();
  SynthFpgaPass.part_name = "generic";
  SynthFpgaPass.no_sdff = true;
  SynthFpgaPass.setup_options();
  WB_CHECK(has("$_DFF_PN?_"));
  WB_CHECK(!has("$_SDFF_P??_"));
}

// -opt is always area, delay or fast after check_options(); with an empty
// mode abc_synthesize() falls back to a plain 'abc -lut'.
WB_TEST(synth_fpga_abc_without_mode) {
  RTLIL::Design *design = yosys_get_design();
  load(design, "module top(input [3:0] a, output y); assign y = ^a; endmodule");
  Pass::call(design, "synth -run begin:fine");
  SynthFpgaPass.active_design = design;
  SynthFpgaPass.opt = "";
  SynthFpgaPass.sc_syn_lut_size = "4";
  wbtest::LogCapture log;
  SynthFpgaPass.abc_synthesize();
  WB_CHECK(log.contains("Performing OFFICIAL PLATYPUS optimization"));
  WB_CHECK(GetSize(design->top_module()->cells()) > 0);
}

// The flow always ties undriven nets to 'x'; the report-only mode is unused.
WB_TEST(synth_fpga_undriven_report_only) {
  RTLIL::Design *design = yosys_get_design();
  RTLIL::Module *top = load(design, "module top(input a, output y, output z); assign y = a; endmodule");
  wbtest::LogCapture log;
  SynthFpgaPass.stop_if_undriven_nets = false;
  SynthFpgaPass.analyze_undriven_nets(top, false);
  WB_CHECK(log.contains("is undriven"));
  WB_CHECK(!log.contains("Setting to 'X'"));
}

WB_TEST(synth_fpga_dff_counter_sync_set_reset_cells) {
  RTLIL::Design *design = yosys_get_design();
  Pass::call(design, "design -reset");
  RTLIL::Module *top = design->addModule(ID(top));
  top->set_bool_attribute(ID::top);
  top->addCell(ID(f0), ID(dffh));
  top->addCell(ID(f1), ID(dffehl));
  top->addCell(ID(f2), ID(dffer));
  WB_CHECK_EQ(SynthFpgaPass.getNumberOfDffs(), 3);
}

WB_TEST(synth_fpga_csv_without_top) {
  RTLIL::Design *design = yosys_get_design();
  Pass::call(design, "design -reset");
  design->addModule(ID(m1));
  design->addModule(ID(m2));
  WB_CHECK(design->top_module() == nullptr);
  wbtest::LogCapture log;
  SynthFpgaPass.dump_csv_file("unused.csv", 0);
  WB_CHECK(log.contains("Design seems empty"));
}

// Any '$' cell left at the end is illegal.  The listing stops after 11 cells
// (KNOWN-ISSUE SF12: the cap is meant to be 10) and synth_fpga stops.
WB_TEST(synth_fpga_illegal_cells) {
  RTLIL::Design *design = yosys_get_design();
  Pass::call(design, "design -reset");
  RTLIL::Module *top = design->addModule(ID(top));
  top->set_bool_attribute(ID::top);
  RTLIL::Wire *a = top->addWire(ID(a)), *b = top->addWire(ID(b));
  for (int i = 0; i < 13; i++)
    top->addAnd(stringf("\\g%d", i), a, b, top->addWire(stringf("\\y%d", i)));
  wbtest::LogCapture log;
  SynthFpgaPass.continue_if_latch = false;
  WB_CHECK_LOG_ERROR(SynthFpgaPass.check_illegal_cells());
  const std::string text = log.text();
  size_t listed = 0;
  for (size_t pos = 0; (pos = text.find("Found illegal cell", pos)) != std::string::npos; pos++)
    listed++;
  WB_CHECK_EQ(listed, size_t(11));
  WB_CHECK(log.contains("some illegal cells found in the design"));
}

// getHeight() and getLeavesIds() are only ever given $_XOR_ cells; anything
// else is an internal error.
WB_TEST(synth_fpga_xor_walk_rejects_other_cells) {
  RTLIL::Design *design = yosys_get_design();
  RTLIL::Module *top = load(design, "module top(input a, b, output y); assign y = a & b; endmodule");
  Pass::call(design, "techmap");
  Cell *and_cell = nullptr;
  for (auto cell : top->cells())
    and_cell = cell;
  WB_CHECK(and_cell != nullptr);
  dict<RTLIL::SigSpec, Cell *> y2xor;
  RTLIL::SigSpec y = and_cell->getPort(ID::Y);
  y2xor[y] = and_cell;
  WB_CHECK_LOG_ERROR(SynthFpgaPass.getHeight(y, y2xor));
  SynthFpgaPass::xor_head xh;
  WB_CHECK_LOG_ERROR(SynthFpgaPass.getLeavesIds(y, y2xor, &xh));
}
