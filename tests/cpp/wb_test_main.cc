// The 'wb_cxxtest' pass: runs the C++ test cases registered with WB_TEST.
#include "wb_test.h"

USING_YOSYS_NAMESPACE
PRIVATE_NAMESPACE_BEGIN

struct WbCxxTestPass : public Pass {
  WbCxxTestPass()
      : Pass("wb_cxxtest", "run the wildebeest C++ unit tests") {}

  void help() override {
    log("\n");
    log("    wb_cxxtest [-list] [-case <name>]\n");
    log("\n");
    log("Runs one registered C++ test case (-case), lists them (-list), or\n");
    log("runs all of them in this process (no option; for local use only:\n");
    log("ctest runs each case in its own process).\n");
    log("\n");
  }

  static bool run_case(const std::string &name, wbtest::CaseFn fn) {
    log("wb_cxxtest: running %s\n", name.c_str());
    try {
      fn();
    } catch (const wbtest::Failure &e) {
      log("wb_cxxtest: FAILED %s: %s\n", name.c_str(), e.what());
      return false;
    } catch (const wbtest::LogError &) {
      log("wb_cxxtest: FAILED %s: unexpected log_error()\n", name.c_str());
      return false;
    } catch (const std::exception &e) {
      log("wb_cxxtest: FAILED %s: unexpected exception: %s\n", name.c_str(),
          e.what());
      return false;
    }
    log("wb_cxxtest: PASSED %s\n", name.c_str());
    return true;
  }

  void execute(std::vector<std::string> args, RTLIL::Design *) override {
    std::string only;
    bool list = false;
    size_t argidx;
    for (argidx = 1; argidx < args.size(); argidx++) {
      if (args[argidx] == "-case" && argidx + 1 < args.size()) {
        only = args[++argidx];
        continue;
      }
      if (args[argidx] == "-list") {
        list = true;
        continue;
      }
      break;
    }
    if (argidx != args.size())
      log_cmd_error("Unknown option: %s\n", args[argidx].c_str());

    auto &cases = wbtest::registry();
    if (list) {
      for (auto &it : cases)
        log("%s\n", it.first.c_str());
      return;
    }
    if (!only.empty()) {
      auto it = cases.find(only);
      if (it == cases.end())
        log_cmd_error("No test case named '%s'.\n", only.c_str());
      if (!run_case(it->first, it->second))
        log_cmd_error("Test case '%s' failed.\n", only.c_str());
      return;
    }
    int failed = 0;
    for (auto &it : cases)
      failed += !run_case(it.first, it.second);
    if (failed)
      log_cmd_error("%d of %d test cases failed.\n", failed, GetSize(cases));
  }
} WbCxxTestPass;

PRIVATE_NAMESPACE_END
