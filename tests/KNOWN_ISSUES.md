# Known issues found while building the test suite

The test suite treats current behaviour as the expected result. Problems
found along the way are listed here for review rather than fixed. A test that
pins down one of these behaviours has a `# KNOWN-ISSUE <ID>` comment, so
`grep -rn KNOWN-ISSUE tests/` lists every test that will need updating when a
fix lands.

Bugs that throw an uncaught C++ exception (ML1, MH2, MH5) cannot be tested
from a script, because they abort Yosys; the C++ unit tests in `tests/cpp/`
pin them with `WB_CHECK_THROWS`.

The exception is a confirmed **wrong-logic** bug: pinning a wrong netlist as
"expected" would be misleading, so such a test asserts the correct behaviour
instead and has a `<script>.xfail` file next to it. CTest then expects it to
fail (label `xfail`); once the bug is fixed the test starts failing, which is
the cue to delete the `.xfail` file.

Severity: **H** = wrong result or crash, **M** = wrong behaviour or
diagnostic, **L** = cosmetic. "To confirm" means the issue was found by
reading the code and still needs a reproducing test.

Line numbers refer to the source as of commit `6763981`.

## synth_fpga (`src/synth_fpga.cc`, `src/abc_scripts/`)

| ID | Sev | Where | Issue |
|----|-----|-------|-------|
| SF1 | M | 4048-4054 | The top-module check runs before `hierarchy`. A design with several modules returns "Design seems empty", even with `-top`. |
| SF2 | H | 2760 | `run("blackbox ")` has no arguments and blackboxes the whole design. With `-obs_clean` on z1010 (the default part) the top module becomes a blackbox, `top_module()` returns null, and `synth_fpga` **segfaults** in `analyze_undriven_nets(nullptr)` (confirmed in Phase 2c). `-obs_clean` works on z1000, where this line does not run. |
| SF3 | M | 2797, 4089 | `-no_flatten -obs_clean` reads the FF models a second time, giving a "Re-definition of module" error. |
| SF4 | M | 2940 | A missing space builds the command `dfflegalize-cell`. Triggered by an empty `legalize_list` in a config file. |
| SF5 | M | 2896 | `-opt fast` with LUT4 or LUT6 points at `.scr` files that don't exist. `-abc_script_version` is never validated. |
| SF6 | M | globals | Config and flop state (`G_config`, `ys_legal_flops`, ...) is never reset between two `synth_fpga` runs in one session. |
| SF7 | H, **confirmed** | 1534, 2052 | XOR-tree leaves are kept in a set, so a repeated leaf doesn't cancel. `(a8^a9)^(a8^a10)^(a11^a12)` synthesized with `-opt delay` computes `a8^a9^a10^a11^a12` instead of `a9^a10^a11^a12` (SAT counterexample; the same design is proven equivalent with `-no_xor_tree_process`, in area mode, and with Yosys's own `synth -lut 4`). Test `synth_fpga/flow/flow-opt_delay-repeated_xor_leaf.ys` asserts equivalence and is marked `.xfail`. |
| SF8 | M | 3403 | The DSP mode map uses the command-line partname, not the partname in the config file. |
| SF9 | L | 711-725 | The DFF-feature fallback flags are inverted. |
| SF10 | L | 2660 | The CSV logic level is -1 when `-show_max_level` is given. |
| SF11 | L | 3595 | The `-use_bram_tech` help text says "DSP"; `bram_tech` has no effect at all. |
| SF12 | L | various | The EOF error doesn't name the file (145). An extra 1 s is added to the run time (4349). `leaf_info` and `A2xors` leak memory. The illegal-cell list shows 11 cells, not 10. |
| SF13 | L | tests | `heartbeat-z1010-hardcode.ys` actually runs `-partname z1000`. Kept as the z1000 test; `heartbeat-z1010-hardcode-real.ys` covers z1010. |
| SF14 | H | `abc_scripts/*.scr` | `LUT4/BEST/tiny_area_lut4.scr` and other scripts using `write_blif`/`read_blif` write a fixed-name file such as `tiny_area_lut4.blif` into the current directory and read it back. Two runs in the same directory corrupt each other's ABC input and segfault. The test runner works around it with one sandbox directory per test. |

| SF15 | M | 594-701, 2710-2746 | `root_path` is applied inconsistently. `setup_options` builds the techmap/libmap paths as `root_path + "/" + file`, and a relative `root_path` is taken relative to the **current directory**, not the config file. `load_cell_models_from_config` reads the flip-flop and DSP model files relative to the **config file's directory** and ignores `root_path`. Any `root_path` other than one naming the config file's own directory (relative to where Yosys runs) therefore fails with "file not found". Found in Phase 2b. |
| SF16 | H | flow | `synth_fpga -no_flatten` segfaults inside Yosys's `stat` (`hierarchy_builder` → `statdata_t`) after the flow has transformed the design, on every design tried, including a single-module counter; `-obs_clean -no_flatten` crashes the same way. Likely related to the cell-model modules the flow reads into the design (they would be flattened away otherwise), but the root cause is not analysed. Seen on Yosys 0.70; to be confirmed on the CI versions. Found in Phase 2c. |
| SF17 | L | `resynthesize` | `-resynthesis` leaves 7 internal nets of the techmapped adder undriven. They drive nothing (the netlist is correct and has no `x`), but `-stop_if_undriven_nets` treats them as fatal. Found in Phase 2c. |
| SF19 | L | 1919-1931 | `show_sig` loops from `sig.chunks().rbegin()` to `sig.chunks().rend()`. Since Yosys 0.70 `SigSpec::chunks()` returns a temporary view, so the two iterators belong to different objects and the loop crashes. `show_sig` is never called by the plugin (dead code); found by the C++ unit tests. |
| SF18 | L | 788-796 | On z1000, `-no_dff_enable`, `-no_dff_async_reset` and `-dff_async_set` have no effect (same netlist as without them); they only change the legal flop list on the platypus parts. Found in Phase 2c. |

## zopt_dff / zopt_const_dff

| ID | Sev | Where | Issue |
|----|-----|-------|-------|
| OD1 | H, to confirm | `zopt_dff.cc:978` | `ff.emit()` in `run_constbits` frees cells that the ModWalker and SAT helper still point to: a possible use-after-free. A sanitizer (ASan) CI job would confirm it. |
| OD2 | L | `zopt_dff.cc:1104` | 1 s is added to the reported run time. |
| OC1 | M | `zopt_const_dff.cc:285` | Every argument is swallowed, so a selection is ignored and bad options are accepted. |
| OC2 | H | `zopt_const_dff.cc:194, 201, 213` | With no init value, `Sx` counts as true: a spurious "conflicting init" warning, and Q may be tied to x. |
| OC3 | H | `zopt_const_dff.cc:155-173` | The reset value is never compared with D, so a flop that resets to 1 with D=0 is replaced by constant 0. |
| OC4 | L | `zopt_const_dff.cc:261` | The help text calls the command `opt_const_dff`. |

## zeroasic_dsp

| ID | Sev | Where | Issue |
|----|-----|-------|-------|
| DSP7 | H, **confirmed** | `zeroasic_dsp.cc:136-142` | For `y = a*b + c_reg` with only C registered, `synth_fpga -partname z1010` absorbs the 36-bit C register into `efpga_mult_addc`. That cell is purely combinational (no clock input) and its `c` port is wired straight to the primary input, so the netlist computes `a*b + c`: one cycle of latency on C is lost. Test `passes/zeroasic_dsp/zeroasic_dsp-mult_addc-c_reg.ys` asserts the correct behaviour and is marked expected-to-fail (`.xfail`). |
| DSP8 | M | `zeroasic_dsp.cc:118-130` | The multiplier path takes its reset from ffA only and never checks that B, C, P and the pipeline register share it. |
| DSP9 | H | `zeroasic_dsp.pmg:345-363` | `out_dffe` doesn't check the reset value, so a P register that resets to non-zero is still packed. |
| DSP10 | M | `zeroasic_dsp.cc:86-97` | The init check covers the whole Q, not the matched slice, so it can trip the assert. It also clears init on flops that stay in the fabric. |
| DSP11 | L | various | Missing null check at pmg:326. Misleading "ffA:" log. `$dffr` is not a real cell type. Dead `$adffe`/`$dffe` arms. |

## Analysis passes

| ID | Sev | Where | Issue |
|----|-----|-------|-------|
| ML1 | H | `clk_domains.cc:342, 372` | With a mixed selection, `bits.at()` throws `std::out_of_range` and crashes Yosys. |
| ML2 | M | `clk_domains.cc:558-560` | Each run adds LUT blackbox modules, and `hierarchy -auto-top` deletes unused modules. `max_height` does the same. |
| ML3 | L | `clk_domains.cc:324, 277` | Returns -1 after storing 0. `bit2ff` keeps only the last FF per bit. |
| MH1 | M | `cp.cc:598` | Selections and unknown options are silently ignored. |
| MH2 | H | `cp.cc:121` | `bits.at()` crashes on a partial selection. |
| MH3 | M | `cp.cc:224` | A combinational LUT loop can end in `log_error`, depending on iteration order. |
| MH4 | L | `cp.cc:95` | Memory leak. |
| MH5 | H | `cp.cc` (`dump_cp_dot`) | `max_height -dot` aborts Yosys with an uncaught `std::out_of_range` (`dict::at()`) when a LUT on the critical path has a constant input. Without `-dot` the same design is reported correctly. Found in Phase 2a. |
| OB1 | H | `obs_clean.cc:275` | With a partial selection, cells outside the selection are deleted. |
| OB2 | M | `obs_clean.cc:251` | The per-bit check isn't sigmapped, so `-wires` may remove a wire that is still connected. |
| OB3 | L | `obs_clean.cc:42, 59, 84` | Unused maps; a raw-vs-sigmapped key mismatch. |
| RS1 | M | `report_stat.cc` | Arguments are silently ignored. `-dot` is undocumented. LUT branches 74/82/100 can never be reached. SB_DFF* counting differs from `max_level`. |
| LM1 | H | `load_models.cc:76, 79` | `load_models` always fails: `ff_models/dffrs.v` and `dffers.v` don't exist. |
| TC1 | L | `time_chrono.cc:117-128` | `-start -end` stores only start. `strftime("%s")` doesn't work on Windows. |
| ZS1 | L | `zsimplemap.cc:496-501` | The help text leaves out about 10 supported cell types. |
