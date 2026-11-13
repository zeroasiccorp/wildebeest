// C++ unit tests for src/time_chrono.cc.
#include "../../src/time_chrono.cc"

#include "wb_test.h"

USING_YOSYS_NAMESPACE

WB_TEST(time_chrono_null_design_guard) {
  TimeChronoPass.G_design = nullptr;
  wbtest::LogCapture log;
  TimeChronoPass.script();
  WB_CHECK(log.contains("Design seems empty"));
}
