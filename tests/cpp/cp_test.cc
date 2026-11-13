// C++ unit tests for src/cp.cc (max_height).
#include "../../src/cp.cc"

#include "wb_test.h"

USING_YOSYS_NAMESPACE

WB_TEST(max_height_null_design_guard) {
  MaxHeigthPass.G_design = nullptr;
  wbtest::LogCapture log;
  MaxHeigthPass.script();
  WB_CHECK(log.contains("Design seems empty"));
}
