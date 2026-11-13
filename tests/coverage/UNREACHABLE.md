# Code the test suite does not cover, and why

Coverage with the script tests and the C++ unit tests (`tests/cpp/`),
measured locally on Yosys 0.70: **93.2% of lines, 65.0% of branches**
(gcovr, `gcovr.cfg`).  Line numbers refer to the source as of commit
`6763981` (no `src/` file has changed since).

Every uncovered line falls into one of the categories below.  The only
category that is real future work is the last one, "Reachable, not yet
covered".

Branch coverage is lower than line coverage mainly because of Yosys header
code inlined into these files (hash tables, `IdString`, `log_*` helpers):
gcov counts branches in that code against the line that uses it, and most of
them can never be taken from this plugin.

## Reached only by the C++ unit tests

For reference, these used to be in the categories below; `tests/cpp/` now
covers them by calling the functions directly:

- All "no design" guards in `synth_fpga.cc`, `cp.cc`, `report_stat.cc`,
  `time_chrono.cc`.
- `synth_fpga.cc` internal errors 1499, 1554, 1625; the never-called helpers
  (`cmpInvHeight`, `id`, `show_const`, `show_sigchunk`, `show_sig`); the
  generic legal-flop list and `-no_sdff` (797-813); `opt == ""` (2825);
  `analyze_undriven_nets(..., false)` (2237); the illegal-cell error and
  listing (2364-2417, except the parts below).
- `zqcsat.cc` `importSig`, `max_cell_outs`, `max_cell_count`.
- The `log_assert`s in the `zsimplemap.cc` mappers.

## 1. Defensive guards that can never fire

| File | Lines | Why |
|---|---|---|
| `obs_clean.cc` | 339 | `execute()` is never called without a design; the C++ test cannot fake it, because `extra_args()` dereferences the design first. |

## 2. Internal consistency errors

| File | Lines | Check |
|---|---|---|
| `cp.cc` | 164-166 | "Problem in the CP extractor" |

## 3. Dead by construction

| File | Lines | Why |
|---|---|---|
| `synth_fpga.cc` | 1402 | Both parameter types `continue` before this line. |
| `synth_fpga.cc` | 2365-2366, 2390, 2394-2411 | The illegal-cell check's exact-name list is empty (its only entry is commented out), and the substring list holds only `$`, so the "..." line after the cap is never printed. |
| `synth_fpga.cc` | 1921-1931 | `show_sig` with several chunks: skipped on Yosys 0.70, where it crashes (KNOWN-ISSUE SF19); covered on older Yosys. |
| `synth_fpga.cc` | 1895 | `show_sigchunk` for a multi-bit slice of an `upto` wire written by `show_sig` only through the multi-chunk path above. |
| `report_stat.cc` | 63, 74-75, 82-84, 100-102 | LUT1..LUT4 and INV are always counted by an earlier `if` (KNOWN-ISSUE RS1). |
| `load_models.cc` | 62 | After `run_script`, which always fails (KNOWN-ISSUE LM1). |
| `zqcsat.cc` | 71 | A cell in a cone without wired inputs; `modwalker` never reports one as a driver. |
| `zeroasic_dsp.cc` | 73-78, 118-121, 162, 222, 227 | The pmg pattern only matches `$dff` / `$adff`, so the enable and `$sdff` arms never see a matching cell. |
| `zopt_const_dff.cc` | 134, 140 | The type-prefix filter already guarantees a built-in, non-empty flip-flop. |
| `zopt_const_dff.cc` | 223, 233-236, 245 | Fine cells are 1 bit wide, so a partial slice never happens. |
| `zopt_dff.cc` | 423-460 | An always-active async load together with an async reset or set/clear: no Yosys cell type has both. |

## 4. Only reached by very large designs

| File | Lines | Needs |
|---|---|---|
| `synth_fpga.cc` | 2857-2891 | More than 50K cells after mapping (small, fast and huge ABC scripts). |
| `synth_fpga.cc` | 3466-3487, 4219-4257 | More than 5M cells (lighter `opt` passes). |
| `synth_fpga.cc` | 3533 | 25K or more coarse cells (`share -fast`).  A test was written: it takes about 3 minutes per run to cover this one line, so it was dropped. |
| `zopt_dff.cc` | 850 | More than 40000 flip-flop bits (largest progress step). |
| `zopt_dff.cc` | 989-990 | More than 5000 cells imported into one SAT solver. |

## 5. Blocked by a known crash

A script test cannot assert a crash; see `tests/KNOWN_ISSUES.md`.

| File | Lines | Issue |
|---|---|---|
| `synth_fpga.cc` | 2761 | SF2: `-obs_clean` on z1010 segfaults right after this line. |
| `synth_fpga.cc` | 3856-3857 | SF16: any run with `-no_flatten` segfaults later in the flow, so its coverage is never written. |

## 6. No input found that reaches it

| File | Lines | Notes |
|---|---|---|
| `synth_fpga.cc` | 3236-3257 | BRAM INIT lost or truncated after techmap.  Needs a techmap or pass that changes INIT later than the existing `bram-init-config-techmap-drops` test does. |
| `synth_fpga.cc` | 3415 | An `MAE` cell left after the DSP mode map. |
| `synth_fpga.cc` | 1690-1700, 1722-1723, 2140-2153 | Duplicate XORs after balancing: `opt_merge` has already removed shared sub-expressions, so trees sharing a leaf pair still show "0 duplicated XORs". |
| `synth_fpga.cc` | 2925-3113, 3173, 3217 | Defensive early returns in the BRAM INIT helpers. |

## 7. gcov artifacts

Lines made only of `cell->type.in(ID(...), ...)`.  Yosys's `ID()` macro
expands to a small function at every use, and gcov counts each of them
separately on that line; the `if` itself is covered.

`report_stat.cc` 121, 160; `zopt_dff.cc` 77; `zopt_const_dff.cc` 78;
`zsimplemap.cc` 256; `zqcsat.cc` 89-108.

## 8. Reachable, not yet covered (future work)

These can be reached with the right input; tests were not written yet.

| File | Lines | What it needs |
|---|---|---|
| `zopt_dff.cc` | 105 | `-keepdc` merging two different constants. |
| `zopt_dff.cc` | 131-146 | A mux tree where the same select appears twice on one feedback path, or `$pmux` with a multi-bit select on the path. |
| `zopt_dff.cc` | 207-224 | Enable logic built for coarse (not fine) cells with several patterns. |
| `zopt_dff.cc` | 247, 665-684 | Chained sync resets of mixed polarity that cannot be folded by `opt_dff` first. |
| `zopt_dff.cc` | 276-278 | A zero-width flip-flop that reaches `zopt_dff` (`opt_clean` removes it first in the current tests). |
| `zopt_dff.cc` | 349, 546-548, 596, 928-939 | Mixed set values, a never-active enable with `-keepdc` and no init, `$sdffce` D == Q with an active-low reset, `-sat` on async-load bits with constant AD. |
| `synth_fpga.cc` | 1516, 1580-1612 | XOR trees whose two sides have equal heights in a different order. |
| `cp.cc` | 202, 211-213, 257 | A constant LUT input printed on the critical path (blocked with `-dot` by MH5). |
| `obs_clean.cc` | 103, 122-127, 185-189, 259-262 | Re-visited cells in the fan-in walk, an assign whose left side is a constant, a wire visited only bit by bit. |
| `clk_domains.cc` | 321-324, 438 | The loop guard in `get_heigth` (probably unreachable) and a fully visited critical-path bit. |
| `zeroasic_dsp.cc` | 316, 322 | Clearing init on a P register whose init is a constant chunk or shorter than the wire. |
