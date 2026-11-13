# Known issues found while building the test suite

The test suite treats current behaviour as the expected result. Problems
found along the way are listed here for review rather than fixed. A test that
pins down one of these behaviours has a `# KNOWN-ISSUE <ID>` comment, so
`grep -rn KNOWN-ISSUE tests/` lists every test that will need updating when a
fix lands.

Severity: **H** = wrong result or crash, **M** = wrong behaviour or
diagnostic, **L** = cosmetic. "To confirm" means the issue was found by
reading the code and still needs a reproducing test.

Line numbers refer to the source as of commit `6763981`.

## synth_fpga (`src/synth_fpga.cc`, `src/abc_scripts/`)

| ID | Sev | Where | Issue |
|----|-----|-------|-------|
| SF1 | M | 4048-4054 | The top-module check runs before `hierarchy`. A design with several modules returns "Design seems empty", even with `-top`. |
| SF2 | H | 2760 | `run("blackbox ")` has no arguments and blackboxes the whole design, so `-obs_clean` on z1010 empties the netlist. |
| SF3 | M | 2797, 4089 | `-no_flatten -obs_clean` reads the FF models a second time, giving a "Re-definition of module" error. |
| SF4 | M | 2940 | A missing space builds the command `dfflegalize-cell`. Triggered by an empty `legalize_list` in a config file. |
| SF5 | M | 2896 | `-opt fast` with LUT4 or LUT6 points at `.scr` files that don't exist. `-abc_script_version` is never validated. |
| SF6 | M | globals | Config and flop state (`G_config`, `ys_legal_flops`, ...) is never reset between two `synth_fpga` runs in one session. |
| SF7 | H, to confirm | 1534, 2052 | XOR-tree leaves are kept in a set, so duplicate leaves don't cancel. Possibly wrong logic with `-opt delay`. |
| SF8 | M | 3403 | The DSP mode map uses the command-line partname, not the partname in the config file. |
| SF9 | L | 711-725 | The DFF-feature fallback flags are inverted. |
| SF10 | L | 2660 | The CSV logic level is -1 when `-show_max_level` is given. |
| SF11 | L | 3595 | The `-use_bram_tech` help text says "DSP"; `bram_tech` has no effect at all. |
| SF12 | L | various | The EOF error doesn't name the file (145). An extra 1 s is added to the run time (4349). `leaf_info` and `A2xors` leak memory. The illegal-cell list shows 11 cells, not 10. |
| SF13 | L | tests | `heartbeat-z1010-hardcode.ys` actually runs `-partname z1000`. Kept as the z1000 test; `heartbeat-z1010-hardcode-real.ys` covers z1010. |
| SF14 | H | `abc_scripts/*.scr` | `LUT4/BEST/tiny_area_lut4.scr` and other scripts using `write_blif`/`read_blif` write a fixed-name file such as `tiny_area_lut4.blif` into the current directory and read it back. Two runs in the same directory corrupt each other's ABC input and segfault. The test runner works around it with one sandbox directory per test. |

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
| DSP7 | H, to confirm | `zeroasic_dsp.cc:136-142` | With only C registered, `C_REG` may not be honoured by the mode map, so a cycle of latency would be lost. resetn is also left undriven. |
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
| OB1 | H | `obs_clean.cc:275` | With a partial selection, cells outside the selection are deleted. |
| OB2 | M | `obs_clean.cc:251` | The per-bit check isn't sigmapped, so `-wires` may remove a wire that is still connected. |
| OB3 | L | `obs_clean.cc:42, 59, 84` | Unused maps; a raw-vs-sigmapped key mismatch. |
| RS1 | M | `report_stat.cc` | Arguments are silently ignored. `-dot` is undocumented. LUT branches 74/82/100 can never be reached. SB_DFF* counting differs from `max_level`. |
| LM1 | H | `load_models.cc:76, 79` | `load_models` always fails: `ff_models/dffrs.v` and `dffers.v` don't exist. |
| TC1 | L | `time_chrono.cc:117-128` | `-start -end` stores only start. `strftime("%s")` doesn't work on Windows. |
| ZS1 | L | `zsimplemap.cc:496-501` | The help text leaves out about 10 supported cell types. |
