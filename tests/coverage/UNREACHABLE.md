# Code the test suite does not cover, and why

Coverage at the end of Phase 3, measured locally on Yosys 0.70:
**92.3% of lines, 64.4% of branches** (gcovr, `gcovr.cfg`).
Line numbers refer to the source as of commit `6763981` (no `src/` file has
changed since).

Every uncovered line falls into one of the categories below.  The only
category that is real future work is the last one, "Reachable, not yet
covered".

Branch coverage is lower than line coverage mainly because of Yosys header
code inlined into these files (hash tables, `IdString`, `log_*` helpers):
gcov counts branches in that code against the line that uses it, and most of
them can never be taken from this plugin.

## 1. Defensive guards that can never fire

`yosys_get_design()` is never null while a pass runs, and `synth_fpga` only
reaches these points after a top module exists.

| File | Lines |
|---|---|
| `synth_fpga.cc` | 2184, 2263-2265, 2313-2315, 2465-2467, 2585-2587, 2608-2610, 2635-2637, 2645-2647, 2817-2819, 4043 |
| `cp.cc` | 621-622 |
| `obs_clean.cc` | 339 |
| `report_stat.cc` | 511 |
| `time_chrono.cc` | 92 |

## 2. Internal consistency errors

`log_error` calls that only fire if the plugin's own data structures are
inconsistent.

| File | Lines | Check |
|---|---|---|
| `synth_fpga.cc` | 1499, 1554 | "Expected to access a XOR cell" |
| `synth_fpga.cc` | 1625-1626 | XOR tree build with fewer than 2 leaves |
| `cp.cc` | 164-166 | "Problem in the CP extractor" |

## 3. Dead by construction

| File | Lines | Why |
|---|---|---|
| `synth_fpga.cc` | 1402 | Both parameter types `continue` before this line. |
| `synth_fpga.cc` | 2237 | `analyze_undriven_nets` is always called with `connect_to_undef = true`. |
| `synth_fpga.cc` | 2825-2828 | `opt == ""` is impossible: `-opt` is validated to area, delay or fast. |
| `report_stat.cc` | 63, 74-75, 82-84, 100-102 | LUT1..LUT4 and INV are always counted by an earlier `if` (KNOWN-ISSUE RS1). |
| `obs_clean.cc` | 296 | Empty `script()`; the pass overrides `execute()`. |
| `load_models.cc` | 62 | After `run_script`, which always fails (KNOWN-ISSUE LM1). |
| `zqcsat.cc` | 25-29 | `importSig` is never called. |
| `zqcsat.cc` | 67-71, 79 | `max_cell_outs` and `max_cell_count` are always 0. |
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
| `synth_fpga.cc` | 2364-2417 | The illegal-cell error.  Every leftover flip-flop variant tried was mapped by a fallback, latches stop at the earlier latch check, and tristates are removed before this check. |
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
| `synth_fpga.cc` | 2622 | A `$_SDFF*` cell counted by the CSV flip-flop counter. |
| `cp.cc` | 133, 202, 211-213, 257, 440 | A constant LUT input printed on the critical path (blocked with `-dot` by MH5). |
| `obs_clean.cc` | 103, 122-127, 185-189, 259-262 | Re-visited cells in the fan-in walk, an assign whose left side is a constant, a wire visited only bit by bit. |
| `clk_domains.cc` | 321-324, 438 | The loop guard in `get_heigth` (probably unreachable) and a fully visited critical-path bit. |
| `zeroasic_dsp.cc` | 316, 322 | Clearing init on a P register whose init is a constant chunk or shorter than the wire. |

The C++ unit-test layer described in the plan (section 2d) would reach most
of categories 1-3 by calling the functions directly.
