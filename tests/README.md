# Wildebeest tests

Every `*.ys` file under `unit/` is a test (C++ unit tests are described below): a Yosys script that loads the
plugin, runs a pass and checks the result with `select -assert-*`,
`logger -expect`, `scratchpad -assert*`, and SAT/`equiv_opt` equivalence
checks.  `ctest` finds them automatically.

## Running

```sh
cmake -S . -B build && cmake --build build && cmake --install build
cd build
ctest -j$(nproc) -LE slow        # what the build matrix runs
ctest -L zopt_dff                # one group (labels follow the folders)
ctest -R obs_clean-wires -V      # one test, with its Yosys log
```

Each test runs in its own directory under `build/tests/sandbox/`, through
`run_yosys_test.cmake`, so tests can run in parallel and never write into the
source tree.  stdin is empty and `tools/` (a stub `xdot`) comes first on
`PATH`.

## Layout

| Folder | What it tests |
|---|---|
| `unit/check_commands.ys` | `help` for every command |
| `unit/synth_fpga/{heartbeat,dsp,carrychain,bram}/` | End-to-end `synth_fpga` runs per feature |
| `unit/synth_fpga/options/` | Option validation errors |
| `unit/synth_fpga/flow/` | Flow branches selected by options |
| `unit/synth_fpga/{json,config}/` | Config-file parser and reader (generated, see below) |
| `unit/passes/<pass>/` | One pass called directly |
| `unit/data/` | Shared inputs: designs, RTLIL fixtures, config files, techmaps |

## Conventions

- **Current behaviour is the expected result.**  Where that behaviour is a
  known bug, the test says so with a `# KNOWN-ISSUE <ID>` comment; the IDs are
  in `KNOWN_ISSUES.md`.
- **Wrong logic is never pinned as expected.**  For a confirmed wrong-logic
  bug the test asserts the correct result and has a `<test>.ys.xfail` file
  next to it (CTest `WILL_FAIL`, label `xfail`).  When the bug is fixed the
  test starts failing: delete the `.xfail` file.
- **Environment variables** for one test go in `<test>.ys.env`
  (`KEY=VALUE` lines).
- **Slow tests** are named `*-slow.ys`; they get the label `slow` and run only
  in the coverage job.
- **Version-independent checks**: Yosys versions 0.47-0.66 are tested in CI.
  Avoid asserting cell names created by `proc` (`$procdff$NN`) or counts of
  cells that come from `proc`, `simplemap` or ABC; count flip-flops and use
  `-assert-min 1` for LUTs.  Inside quoted `logger` patterns use `.` instead
  of backslash escapes (they do not survive Yosys's script parser), and note
  that a count of 0 is not accepted.
- **Generated tests**: `unit/synth_fpga/{json,config}/` and their fixtures in
  `unit/data/config/` come from the table in
  `unit/data/config/generate.py`.  Edit the table and run
  `python3 tests/unit/data/config/generate.py .` from the repository root.

## C++ unit tests (`cpp/`)

A small layer for code that scripts cannot reach: "no design" guards,
internal errors, helpers nothing calls, and bugs that abort Yosys with an
uncaught exception.  Behaviour is still tested through scripts.

- Each `cpp/<name>_test.cc` does `#include "../../src/<name>.cc"`, so it can
  use the pass object and members that live in the source's anonymous
  namespace.  CMake builds `wildebeest_cxxtest.so` from the test files plus
  the remaining sources; it is loaded *instead of* `wildebeest.so`.
- A case is `WB_TEST(name) { ... }` with `WB_CHECK`, `WB_CHECK_EQ`,
  `WB_CHECK_THROWS` and `WB_CHECK_LOG_ERROR` (see `cpp/wb_test.h`).
  `WB_CHECK_LOG_ERROR` works by making Yosys's `log_error_atexit` hook
  throw.  `wbtest::NullDesign` and `wbtest::LogCapture` help reach guards and
  check log output.
- CTest runs each case as `yosys -m wildebeest_cxxtest.so -p "wb_cxxtest
  -case <name>"` in its own process and directory (label `cxx`), so a case
  may leave Yosys in any state.
- Built with `-DWILDEBEEST_CXXTESTS=ON`, which is the default in coverage
  builds (`-DWILDEBEEST_COVERAGE=ON`); the regular build matrix does not
  build them.
- The tests depend on private names in `src/`; when a test stops compiling
  after a refactor, update or delete it.

## Coverage

Configure with `-DWILDEBEEST_COVERAGE=ON`, run `ctest`, then from the
repository root:

```sh
gcovr --object-directory build              # summary (settings in gcovr.cfg)
python3 tests/coverage/check_ratchet.py summary.json   # after --json-summary summary.json
```

CI (`.github/workflows/build.yml`, jobs `coverage` and `coverage-report`)
measures on Yosys v0.57 and v0.66, merges the results, posts an HTML report,
a job summary and a PR comment, and fails if coverage drops below
`coverage/thresholds.json`.  Raise those numbers when a change adds coverage.
`coverage/UNREACHABLE.md` lists every line that is not covered and why.
