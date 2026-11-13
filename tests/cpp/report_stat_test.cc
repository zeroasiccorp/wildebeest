// C++ unit tests for src/report_stat.cc.
#include "../../src/report_stat.cc"

#include "wb_test.h"

USING_YOSYS_NAMESPACE

WB_TEST(report_stat_null_design_guard) {
  ReportStatPass.G_design = nullptr;
  wbtest::LogCapture log;
  ReportStatPass.script();
  WB_CHECK(log.contains("Design seems empty"));
}
