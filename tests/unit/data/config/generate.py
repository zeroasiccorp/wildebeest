"""Generate the synth_fpga config-file tests.

Writes one fixture per case to tests/unit/data/config/<group>-<name>.json and
one test to tests/unit/synth_fpga/<group>/<group>-<name>.ys, from the CASES
table below.  Both the generated fixtures and tests are committed; to add or
change a case, edit the table and re-run from the repository root:

    python3 tests/unit/data/config/generate.py .

Error cases expect one log_error and stop there.  Cases marked runs_flow run
the whole synth_fpga flow and check the warnings it gives.
"""
import copy
import json
import os
import re
import sys

ROOT = sys.argv[1]  # repo root
DATA = os.path.join(ROOT, "tests/unit/data/config")
TESTS = os.path.join(ROOT, "tests/unit/synth_fpga")
os.makedirs(DATA, exist_ok=True)

BASE = {
    "version": 1,
    "partname": "z1000",
    "lut_size": 4,
    "flipflops": {
        "features": ["async_reset", "async_set", "flop_enable"],
        "models": {},
        "legalize_list": ["$_DFF_PN0_", "$_DFF_P_", "$_DFFE_PP_", "$_DFFE_PN0P_"],
        "techmap": "../z1000/tech_flops.v",
    },
}

DESIGN = """read_verilog <<EOF
module top (input clk, nreset, input [3:0] d, output reg [3:0] q);
   always @(posedge clk or negedge nreset)
      if (!nreset) q <= 4'b0000;
      else q <= q + d;
endmodule
EOF"""


def rx(msg):
    """Turn a literal message into a logger regex without backslashes."""
    return re.sub(r"[][(){}.*+?^$|\\\"]", ".", msg)


def mod(**changes):
    """Base config with dotted-path changes; value None deletes the key."""
    c = copy.deepcopy(BASE)
    for path, val in changes.items():
        keys = path.split("__")
        d = c
        for k in keys[:-1]:
            d = d.setdefault(k, {})
        if val is None:
            d.pop(keys[-1], None)
        else:
            d[keys[-1]] = val
    return json.dumps(c, indent=2) + "\n"


CASES = []


def case(group, name, comment, content, expects, opts="", runs_flow=False,
         asserts=None, config=None):
    CASES.append(dict(group=group, name=name, comment=comment, content=content,
                      expects=expects, opts=opts, runs_flow=runs_flow,
                      asserts=asserts or [], config=config))


def err(msg):
    return [("error", rx(msg), 1)]


# ---------------------------------------------------------------- JSON syntax
J = "json"
case(J, "empty_file", "An empty file hits EOF before any value.", "",
     err("Unexpected EOF in JSON file."))
case(J, "eof_in_string", "EOF inside a string.", '{"vers',
     err("Unexpected EOF in JSON string."))
case(J, "eof_after_backslash", "EOF right after a backslash in a string.",
     '{"vers\\', err("Unexpected EOF in JSON string."))
case(J, "unicode_bad_digit", "A non-hex digit in a \\uXXXX escape.",
     '{"a\\u12G4": 1}',
     err("Unexpected non-digit character in ") + [])
case(J, "unicode_non_ascii", "\\uXXXX escapes of 128 and above are refused.",
     '{"a\\u00e9": 1}', err("Unsupported ") )
case(J, "true_eof", "EOF in the middle of 'true'.", '{"a": tr',
     err("Unexpected EOF in JSON file '"))
case(J, "true_misspelled", "'tru' followed by something else.", '{"a": tru }',
     err("Expected 'true' in config file '"))
case(J, "false_misspelled", "'fals' followed by something else.",
     '{"a": fals }', err("Expected 'false' in config file '"))
case(J, "array_eof", "EOF inside an array.", '{"a": [1, 2',
     err("Unexpected EOF in JSON file '"))
case(J, "dict_eof", "EOF inside a dictionary.", '{"a": 1',
     err("Unexpected EOF in JSON file '"))
case(J, "dict_eof_after_key", "EOF right after a dictionary key.", '{"a"',
     err("Unexpected EOF in JSON file '"))
case(J, "non_string_key", "A dictionary key that is not a string.",
     '{1: 2}', err("Unexpected non-string key in JSON dict at line 1."))
case(J, "null_value", "'null' is not supported (unexpected 'n').",
     '{"a": null}', err("Unexpected character 'n' in config file '"))
case(J, "lone_brace", "A closing brace where a value is expected.",
     '}', err("Unexpected character '}' in config file '"))

# Valid JSON features: escapes, negative / real numbers, false, lenient
# separators, extra keys, text after the root value.
esc = "\n  \t" + mod()
esc = esc.replace('"partname": "z1000"',
                  '"partname": "z\\u0031000 \\u004A \\" \\/ \\\\ \\b \\f \\n \\r \\t \\q"')
esc = esc.rstrip("\n").rstrip("}") + \
    ',\n  "extra_newline"\n  :\n  1,' \
    '\n  "extra_negative": -42,\n  "extra_minus": -,\n  "extra_real": -1.25,' \
    '\n  "extra_false": false,\n  "extra_true": true,\n  "extra_list": [1 2 3],' \
    '\n  "extra_dict": {"k" "v" "n": 7}\n}\ntrailing text is ignored\n'
case(J, "valid_features",
     "Leading whitespace, escapes in strings (\\u with lower- and upper-case\n"
     "# hex digits, and an unknown escape), a newline between key and value,\n"
     "# negative and real numbers, a lone '-', true / false, separators left\n"
     "# out, unknown keys and text after the root value are all accepted.",
     esc, [("log", rx("Using config file '"), 0)], runs_flow=True,
     asserts=["select -assert-min 1 t:$lut"])

# ------------------------------------------------------------ config errors
C = "config"
case(C, "missing_file", "A config file that does not exist.", None,
     err("Cannot find file '../../data/config/does_not_exist.json'."),
     config="../../data/config/does_not_exist.json")
case(C, "not_a_dict", "The root value must be a dictionary.", "[1, 2]\n",
     err("file is not a dictionary."))
case(C, "not_a_dict_number", "A bare number (here a real ending at EOF) is not\n"
     "# a dictionary either.", "1.5", err("file is not a dictionary."))
case(C, "version_missing", "'version' is required.", mod(version=None),
     err("'version' number is missing in config file '"))
case(C, "version_string", "'version' must be an integer.", mod(version="1"),
     err("'version' must be an integer."))
case(C, "version_real", "A real number is stored as a string, so 1.0 is not\n"
     "# an integer.", mod().replace('"version": 1,', '"version": 1.0,'),
     err("'version' must be an integer."))
case(C, "root_path_type", "'root_path' must be a string.", mod(root_path=3),
     err("'root_path' must be a string."))
case(C, "partname_missing", "'partname' is required.", mod(partname=None),
     err("'partname' is missing in config file '"))
case(C, "partname_type", "'partname' must be a string.", mod(partname=1000),
     err("'partname' must be a string."))
case(C, "lut_size_missing", "'lut_size' is required.", mod(lut_size=None),
     err("'lut_size' is missing in config file '"))
case(C, "lut_size_type", "'lut_size' must be an integer.", mod(lut_size="4"),
     err("'lut_size' must be an integer."))
case(C, "flipflops_missing", "'flipflops' is required.", mod(flipflops=None),
     err("'flipflops' is missing in config file '"))
case(C, "flipflops_type", "'flipflops' must be a dictionary.",
     mod(flipflops=["x"]), err("'flipflops' must be a dictionnary."))
case(C, "brams_type", "'brams' must be a dictionary.", mod(brams=1),
     err("'brams' must be a dictionnary."))
case(C, "dsps_type", "'dsps' must be a dictionary.", mod(dsps="x"),
     err("'dsps' must be a dictionnary."))
case(C, "features_missing", "'flipflops.features' is required.",
     mod(flipflops__features=None),
     err("'features' from 'flipflops' is missing in config file '"))
case(C, "features_type", "'flipflops.features' must be an array.",
     mod(flipflops__features="async_reset"),
     err("'features' associated to 'flipflops' must be an array."))
case(C, "features_item_type", "'flipflops.features' items must be strings.",
     mod(flipflops__features=["async_reset", 1]),
     err("Array associated to DFF 'features' must contain only strings."))
case(C, "models_missing", "'flipflops.models' is required.",
     mod(flipflops__models=None),
     err("'models' from 'flipflops' is missing in config file '"))
case(C, "models_value_type", "'flipflops.models' values must be strings.",
     mod(flipflops__models={"dff": 1}),
     err("Second element associated to DFF models 'dff' must be a string."))
case(C, "legalize_list_missing", "'flipflops.legalize_list' is required.",
     mod(flipflops__legalize_list=None),
     err("'legalize_list' from 'flipflops' is missing in config file '"))
case(C, "legalize_list_type", "'flipflops.legalize_list' must be an array.",
     mod(flipflops__legalize_list="$_DFF_P_"),
     err("'legalize_list' associated to 'flipflops' must be an array."))
case(C, "legalize_list_item_type",
     "'flipflops.legalize_list' items must be strings.",
     mod(flipflops__legalize_list=["$_DFF_P_", 2]),
     err("Array associated to DFF 'legalize_list' must contain only strings."))
case(C, "ff_techmap_missing", "'flipflops.techmap' is required.",
     mod(flipflops__techmap=None),
     err("'techmap' from 'flipflops' is missing in config file '"))
case(C, "ff_techmap_type", "'flipflops.techmap' must be a string.",
     mod(flipflops__techmap=["x.v"]),
     err("'techmap' associated to 'flipflops' must be a string."))
case(C, "memory_libmap_type", "'brams.memory_libmap' must be an array.",
     mod(brams={"memory_libmap": "x.txt", "techmap": ["x.v"]}),
     err("'memory_libmap' associated to 'brams' must be an array."))
case(C, "memory_libmap_item_type", "'brams.memory_libmap' items must be strings.",
     mod(brams={"memory_libmap": [1], "techmap": ["x.v"]}),
     err("Array associated to 'memory_libmap' must be contain only strings."))
case(C, "memory_libmap_parameters_type",
     "'brams.memory_libmap_parameters' must be an array.",
     mod(brams={"memory_libmap": ["x.txt"], "memory_libmap_parameters": "-x",
                "techmap": ["x.v"]}),
     err("'memory_libmap_parameters' associated to 'brams' must be an array."))
case(C, "memory_libmap_parameters_item_type",
     "'brams.memory_libmap_parameters' items must be strings.",
     mod(brams={"memory_libmap": ["x.txt"], "memory_libmap_parameters": [3],
                "techmap": ["x.v"]}),
     err("Array associated to 'memory_libmap_parameters' must be contain only strings."))
case(C, "logic_cost_rom_type", "'brams.logic_cost_rom' must be a number.",
     mod(brams={"memory_libmap": ["x.txt"], "logic_cost_rom": [1],
                "techmap": ["x.v"]}),
     err("'logic_cost_rom' associated to 'brams' must be a number."))
case(C, "bram_with_init_type", "'brams.bram_with_init' must be a boolean.",
     mod(brams={"memory_libmap": ["x.txt"], "bram_with_init": "yes",
                "techmap": ["x.v"]}),
     err("'bram_with_init' associated to 'brams' must be a boolean."))
case(C, "bram_techmap_type", "'brams.techmap' must be an array.",
     mod(brams={"memory_libmap": ["x.txt"], "techmap": "x.v"}),
     err("'techmap' associated to 'brams' must be an array."))
case(C, "bram_techmap_item_type", "'brams.techmap' items must be strings.",
     mod(brams={"memory_libmap": ["x.txt"], "techmap": [1]}),
     err("Array associated to 'techmap' must be contain only strings."))
case(C, "dsp_family_type", "'dsps.family' must be a string.",
     mod(dsps={"family": 1}), err("'family' associated to 'dsps' must be a string."))
case(C, "dsp_techmap_type", "'dsps.techmap' must be a string.",
     mod(dsps={"family": "x", "techmap": 1}),
     err("'techmap' associated to 'dsps' must be a string."))
case(C, "dsp_techmap_parameters_type",
     "'dsps.techmap_parameters' must be a dictionary.",
     mod(dsps={"family": "x", "techmap": "x.v", "techmap_parameters": [1]}),
     err("'techmap_parameters' associated to 'dsps' must be a dictionnary."))
case(C, "dsp_techmap_parameters_value_type",
     "'dsps.techmap_parameters' values must be strings or integers.",
     mod(dsps={"family": "x", "techmap": "x.v",
               "techmap_parameters": {"A_SIZE": [18]}}),
     err("Second element associated to dsps 'techmap_parameters' 'A_SIZE' must be either a string or an integer."))
case(C, "dsp_pack_command_type", "'dsps.pack_command' must be a string.",
     mod(dsps={"family": "x", "techmap": "x.v", "techmap_parameters": {},
               "pack_command": 1}),
     err("'pack_command' associated to 'dsps' must be a string."))
case(C, "adders_type", "'adders' must be a dictionary.", mod(adders=[1]),
     err("'adders' must be a dictionnary."))
case(C, "adders_techmap_type", "'adders.techmap' must be a string.",
     mod(adders={"techmap": 1}),
     err("'techmap' associated to 'adders' must be a string."))

# ---------------------------------------------- config warnings, flow runs
W = "config"
case(W, "root_path_config_dir", "An explicit 'root_path' naming the config file's own directory\n"
     "# works like no 'root_path' at all.  KNOWN-ISSUE SF15: a relative root_path\n"
     "# is resolved against the current directory, not the config file, so it\n"
     "# must be spelled relative to where yosys runs ('.' would not work).",
     mod(root_path="../../data/config"), [("log", rx("Using config file '"), 0)], runs_flow=True,
     asserts=["select -assert-min 1 t:$lut"])
case(W, "root_path_elsewhere", "KNOWN-ISSUE SF15: 'root_path' is used for the techmap commands but\n"
     "# not when the flip-flop / DSP model files are read, which always uses the\n"
     "# config file's directory.  With root_path pointing elsewhere the model\n"
     "# file is looked up next to the config file and is not found.",
     mod(root_path="../z1000", flipflops__techmap="tech_flops.v"),
     [("error", rx("data/config/tech_flops.v"), 1)])
case(W, "models_not_dict", "'flipflops.models' that is not a dictionary is\n"
     "# ignored with a warning.", mod(flipflops__models=["x"]),
     [("warning", rx("'models' associated to 'flipflops' must be a dictionary to be read in. Ignoring contents."), 1)],
     runs_flow=True, asserts=["select -assert-min 1 t:$lut"])
case(W, "models_entries", "Entries in 'flipflops.models' are read but only\n"
     "# warned about: models must come from 'techmap'.",
     mod(flipflops__models={"dff": "ffs/dff.v", "dffr": "ffs/dffr.v"}),
     [("warning", rx("Not reading dff_models from the flipflops 'models' entry. Please use 'techmap'."), 2),
      ("log", rx("                       dffr ffs/dffr.v"), 1)],
     opts="-show_config", runs_flow=True, asserts=["select -assert-min 1 t:$lut"])
case(W, "features_empty", "With no flip-flop features listed, one warning per\n"
     "# missing feature.  KNOWN-ISSUE SF9: the wording (and logic) is inverted:\n"
     "# a missing feature 'switches off' the option that disables it.",
     mod(flipflops__features=[]),
     [("warning", rx("Config file will switch off '-no_dff_async_reset' option."), 1),
      ("warning", rx("Config file will switch on '-dff_async_set' option."), 1),
      ("warning", rx("Config file will switch off '-no_dff_enable' option."), 1)],
     runs_flow=True, asserts=["select -assert-min 1 t:$lut"])
case(W, "lut_size_override", "A config lut_size different from -lut_size\n"
     "# wins, with a warning.", mod(lut_size=6),
     [("warning", rx("Config file will change lut size value from 4 to 6."), 1)], opts="-lut_size 4", runs_flow=True,
     asserts=["select -assert-min 1 t:$lut"])
case(W, "brams_dsps_absent_sections",
     "'brams' and 'dsps' sections are optional: a missing section warns, then\n"
     "# behaves like an empty one (every key in it warns as missing).", mod(),
     [("warning", "^" + rx("'brams' is missing in config file '"), 1),
      ("warning", "^" + rx("'dsps' is missing in config file '"), 1),
      ("warning", rx("'family' from 'dsps' is missing in config file '"), 1),
      ("warning", rx("'memory_libmap' from 'brams' is missing in config file '"), 1)],
     runs_flow=True, asserts=["select -assert-min 1 t:$lut"])
case(W, "brams_empty", "An empty 'brams' section: missing memory_libmap and\n"
     "# techmap each give two warnings and BRAM inference is off.",
     mod(brams={}),
     [("warning", rx("'memory_libmap' from 'brams' is missing in config file '"), 1),
      ("warning", rx("'techmap' from 'brams' is missing in config file '"), 1),
      ("warning", rx("Assuming that this technology has no BRAM support."), 2)],
     runs_flow=True, asserts=["select -assert-min 1 t:$lut"])
case(W, "dsps_empty", "An empty 'dsps' section: each missing key warns and DSP\n"
     "# inference is off.", mod(dsps={}),
     [("warning", rx("'family' from 'dsps' is missing in config file '"), 1),
      ("warning", rx("'techmap' from 'dsps' is missing in config file '"), 1),
      ("warning", rx("'techmap_parameters' from 'dsps' is missing in config file '"), 1),
      ("warning", rx("'pack_command' from 'dsps' is missing in config file '"), 1),
      ("warning", rx("Assuming that this technology has no DSP support."), 3),
      ("warning", rx("Assuming that this technology has no DSP packing support."), 1)],
     runs_flow=True, asserts=["select -assert-min 1 t:$lut"])
case(W, "brams_options", "memory_libmap_parameters, logic_cost_rom as an integer\n"
     "# and bram_with_init false are accepted.  A logic_cost_rom different from\n"
     "# -logic_cost_rom warns and the config value wins.",
     mod(brams={"memory_libmap": ["../../../../architecture/z1010/bram/bram_memory_map.txt"],
                "memory_libmap_parameters": ["-no-auto-distributed"],
                "logic_cost_rom": 2, "bram_with_init": False,
                "techmap": ["../../../../architecture/z1010/bram/tech_bram.v"]}),
     [("warning", rx("logic_cost_rom"), 1),
      ("log", rx("  BRAM memory_libmap parameters : "), 1),
      ("log", rx("                       -no-auto-distributed"), 1)],
     opts="-logic_cost_rom 0.7 -show_config",
     runs_flow=True, asserts=["select -assert-min 1 t:$lut"])
case(W, "brams_libmap_only", "A BRAM memory_libmap without techmap: warns at\n"
     "# BRAM inference and skips it.",
     mod(brams={"memory_libmap": ["../../../../architecture/z1010/bram/bram_memory_map.txt"]}),
     [("warning", rx("'techmap' from 'brams' is missing in config file '"), 1)],
     runs_flow=True, asserts=["select -assert-min 1 t:$lut"])
case(W, "brams_techmap_only", "A BRAM techmap without memory_libmap: warns at\n"
     "# BRAM inference and skips it.",
     mod(brams={"techmap": ["../../../../architecture/z1010/bram/tech_bram.v"]}),
     [("warning", rx("'memory_libmap' from 'brams' is missing in config file '"), 1)],
     runs_flow=True, asserts=["select -assert-min 1 t:$lut"])
case(W, "dsp_parameters", "dsps.techmap_parameters accepts strings and\n"
     "# integers, and pack_command is any yosys command (here opt_clean).",
     mod(dsps={"family": "DSP48", "techmap": "../z1010/mult18x18_DSP48.v",
               "techmap_parameters": {"DSP_A_MAXWIDTH": 18, "DSP_B_MAXWIDTH": 18,
                                      "DSP_A_MINWIDTH": 2, "DSP_B_MINWIDTH": 2,
                                      "DSP_Y_MINWIDTH": 9, "DSP_SIGNEDONLY": 1,
                                      "DSP_NAME": "$__MUL18X18"},
               "pack_command": "opt_clean"}),
     [("log", rx("Using config file '"), 0)],
     runs_flow=True, asserts=["select -assert-min 1 t:$lut"])

# ------------------------------------------------------------------ writers
for c in CASES:
    d = os.path.join(TESTS, c["group"])
    os.makedirs(d, exist_ok=True)
    fname = f"{c['group']}-{c['name']}"
    cfg = c["config"] or f"../../data/config/{fname}.json"
    if c["content"] is not None:
        with open(os.path.join(DATA, fname + ".json"), "w") as f:
            f.write(c["content"])
    lines = [f"# {c['comment']}", f"# Fixture: data/config/{fname}.json"
             if c["content"] is not None else "# (no fixture: the file is missing)",
             "", DESIGN, ""]
    for kind, pat, n in c["expects"]:
        if n:
            lines.append(f'logger -expect {kind} "{pat}" {n}')
    lines.append(f"synth_fpga -config {cfg} {c['opts']}".rstrip())
    if c["runs_flow"]:
        lines.append("logger -check-expected")
        lines += c["asserts"]
    with open(os.path.join(d, fname + ".ys"), "w") as f:
        f.write("\n".join(lines) + "\n")
print(len(CASES), "cases")
