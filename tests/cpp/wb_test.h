// Minimal unit-test support for the wildebeest C++ test plugin.
//
// The test plugin (wildebeest_cxxtest.so) is the wildebeest plugin with some
// source files replaced by test files that '#include' them, so the tests can
// call functions and set members that live in the sources' anonymous
// namespaces.  Each test case is registered with WB_TEST(name) and run by the
// 'wb_cxxtest -case <name>' pass (see wb_test_main.cc); CTest runs every case
// in its own Yosys process.
//
// Helpers:
//   WB_CHECK(cond), WB_CHECK_EQ(a, b)       fail the case if not true / equal
//   WB_CHECK_THROWS(Type, stmt)             stmt must throw Type
//   WB_CHECK_LOG_ERROR(stmt)                stmt must call log_error()
//   wbtest::NullDesign                      yosys_design is null while alive
//   wbtest::LogCapture                      collects log output while alive
#pragma once

#include "kernel/yosys.h"

#include <fstream>
#include <map>
#include <sstream>
#include <stdexcept>
#include <string>

namespace wbtest {

// A failed check.
struct Failure : std::runtime_error {
  using std::runtime_error::runtime_error;
};

// Thrown instead of exiting when log_error() is called inside an ErrorHook.
struct LogError {};

using CaseFn = void (*)();

inline std::map<std::string, CaseFn> &registry() {
  static std::map<std::string, CaseFn> cases;
  return cases;
}

struct Register {
  Register(const char *name, CaseFn fn) {
    if (!registry().emplace(name, fn).second)
      throw std::logic_error(std::string("duplicate test case: ") + name);
  }
};

// Yosys calls log_error_atexit() right before it exits on log_error().
// Throwing from there turns log_error() into a catchable exception.
// The Yosys state after such an error is not guaranteed to be consistent, so
// a case must not keep using the design after catching one (each case runs
// in its own process).
inline void throw_log_error() { throw LogError(); }

struct ErrorHook {
  void (*previous)();
  ErrorHook() : previous(Yosys::log_error_atexit) {
    Yosys::log_error_atexit = throw_log_error;
  }
  ~ErrorHook() { Yosys::log_error_atexit = previous; }
};

// Makes yosys_get_design() return null, to reach "no design" guards.
struct NullDesign {
  Yosys::RTLIL::Design *previous;
  NullDesign() : previous(Yosys::yosys_design) { Yosys::yosys_design = nullptr; }
  ~NullDesign() { Yosys::yosys_design = previous; }
};

// One stream that receives all log output from the first LogCapture on.
// Yosys 0.70 replaced the 'log_streams' vector by LogManager sinks, which
// cannot be removed again; the stream therefore lives for the whole process
// (one test case per process) and each LogCapture remembers where it began.
inline std::ostringstream &capture_stream() {
  static std::ostringstream *stream = [] {
    auto *s = new std::ostringstream;
#if YOSYS_MAJOR == 0 && YOSYS_MINOR < 70
    Yosys::log_streams.push_back(s);
#else
    Yosys::logger().add_sink<Yosys::StreamLogSink>(*s);
#endif
    return s;
  }();
  return *stream;
}

// Collects everything logged while it is alive.
struct LogCapture {
  size_t start;
  LogCapture() : start(capture_stream().str().size()) {}
  std::string text() const { return capture_stream().str().substr(start); }
  bool contains(const std::string &needle) const {
    return text().find(needle) != std::string::npos;
  }
};

// Resets the current design to the module(s) in 'verilog' (written to a file
// in the working directory, since a here-document cannot be passed to
// Pass::call) and returns the top module.
inline Yosys::RTLIL::Module *load(Yosys::RTLIL::Design *design,
                                  const std::string &verilog,
                                  const std::string &read_options = "") {
  const std::string file = "wbtest_input.v";
  {
    std::ofstream f(file);
    f << verilog << "\n";
  }
  Yosys::Pass::call(design, "design -reset");
  Yosys::Pass::call(design, "read_verilog " + read_options + " " + file);
  Yosys::Pass::call(design, "hierarchy -auto-top");
  return design->top_module();
}

inline std::string where(const char *file, int line) {
  return std::string(file) + ":" + std::to_string(line) + ": ";
}

} // namespace wbtest

#define WB_TEST(name)                                                          \
  static void wbtest_case_##name();                                            \
  static wbtest::Register wbtest_register_##name(#name, wbtest_case_##name);   \
  static void wbtest_case_##name()

#define WB_CHECK(cond)                                                         \
  do {                                                                         \
    if (!(cond))                                                               \
      throw wbtest::Failure(wbtest::where(__FILE__, __LINE__) +               \
                            "CHECK(" #cond ") failed");                       \
  } while (0)

#define WB_CHECK_EQ(a, b)                                                      \
  do {                                                                         \
    auto wb_a_ = (a);                                                          \
    auto wb_b_ = (b);                                                          \
    if (!(wb_a_ == wb_b_)) {                                                   \
      std::ostringstream wb_msg_;                                              \
      wb_msg_ << wbtest::where(__FILE__, __LINE__) << "CHECK_EQ(" #a ", " #b  \
              << ") failed: " << wb_a_ << " != " << wb_b_;                     \
      throw wbtest::Failure(wb_msg_.str());                                    \
    }                                                                          \
  } while (0)

#define WB_CHECK_THROWS(Type, stmt)                                            \
  do {                                                                         \
    bool wb_thrown_ = false;                                                   \
    try {                                                                      \
      stmt;                                                                    \
    } catch (const Type &) {                                                   \
      wb_thrown_ = true;                                                       \
    }                                                                          \
    if (!wb_thrown_)                                                           \
      throw wbtest::Failure(wbtest::where(__FILE__, __LINE__) +               \
                            "expected " #Type " from: " #stmt);               \
  } while (0)

#define WB_CHECK_LOG_ERROR(stmt)                                               \
  do {                                                                         \
    wbtest::ErrorHook wb_hook_;                                                \
    WB_CHECK_THROWS(wbtest::LogError, stmt);                                   \
  } while (0)
