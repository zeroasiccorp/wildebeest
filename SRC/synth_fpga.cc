//
// Zero Asic Corp. Plug-in
//
/*
 *  yosys -- Yosys Open SYnthesis Suite
 *
 *  Copyright (C) 2012  Claire Xenia Wolf <claire@yosyshq.com>
 *
 *  Permission to use, copy, modify, and/or distribute this software for any
 *  purpose with or without fee is hereby granted, provided that the above
 *  copyright notice and this permission notice appear in all copies.
 *
 *  THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 *  WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 *  MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
 *  ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 *  WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
 *  ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
 *  OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 *
 */

#include "kernel/register.h"
#include "kernel/celltypes.h"
#include "kernel/rtlil.h"
#include "kernel/log.h"
#include "version.h"
#include <chrono>

#define SYNTH_FPGA_VERSION "1.0-" YOSYS_SYN_REVISION

USING_YOSYS_NAMESPACE
PRIVATE_NAMESPACE_BEGIN

struct SynthFpgaPass : public ScriptPass
{
  // Global data
  //
  RTLIL::Design *G_design = NULL; 
  string top_opt, verilog_file, part_name, opt;
  string abc_script_version;
  bool no_flatten, dff_enable, dff_async_set, dff_async_reset;
  bool obs_clean, wait, show_max_level, csv, insbuf, resynthesis, autoname;
  bool dsp48, seq_opt;
  string sc_syn_lut_size;

  pool<string> opt_options  = {"default", "fast", "area", "delay"};
  pool<string> partnames  = {"Z1000", "Z1010"};

  // Methods
  //
  SynthFpgaPass() : ScriptPass("synth_fpga", "Zero Asic FPGA synthesis flow") { }

  // -------------------------
  // getNumberOfLuts
  // -------------------------
  int getNumberOfLuts() {

     int nb = 0;

     if (!G_design) {
       log_warning("Design seems empty !\n");
       return -1;
     }

     for (auto cell : G_design->top_module()->cells()) {
         if (cell->type.in(ID($lut))) {
           nb++;
         }
     }

     return nb;
  }

  // -------------------------
  // getNumberOfDffs
  // -------------------------
  int getNumberOfDffs() {

     int nb = 0;

     if (!G_design) {
       log_warning("Design seems empty !\n");
       return -1;
     }

     for (auto cell : G_design->top_module()->cells()) {
         if (cell->type.in(ID(dff), ID(dffe), ID(dffr), ID(dffer),
                           ID(dffs), ID(dffrs), ID(dffes), ID(dffers))) {
           nb++;
         }
     }

     return nb;
  }

  // -------------------------
  // dump_csv_file 
  // -------------------------
  void dump_csv_file(string fileName, int runTime)
  {
     if (!G_design) {
       log_warning("Design seems empty !\n");
       return;
     }

     // -----
     // Get all the stats 
     //
     Module* topModule = G_design->top_module();

     if (!topModule) {
       log_warning("Design seems empty !\n");
       return;
     }

     string topName = log_id(topModule->name);

     int nbLuts = getNumberOfLuts();

     int nbDffs = getNumberOfDffs();

     int maxlvl = -1;

     // call 'max_level' command if not called yet
     //
     if (!show_max_level) {
         run("max_level -summary"); // -> store 'maxlvl' in scratchpad 
	                            // with 'max_level.max_levels'

	 maxlvl = G_design->scratchpad_get_int("max_level.max_levels", 0);
     }

     // -----
     // Open the csv file and dump the stats.
     //
     std::ofstream csv_file(fileName);

     csv_file << topName + ",";
     csv_file << std::to_string(nbLuts) + ",";
     csv_file << std::to_string(nbDffs) + ",";
     csv_file << std::to_string(maxlvl) + ",";
     csv_file << std::to_string(runTime);
     csv_file << std::endl;

     csv_file.close();

     log("\n   Dumped file %s\n", fileName.c_str());
  }

  // -------------------------
  // load_dff_bb_models 
  // -------------------------
  void load_dff_bb_models()
  {
     run("read_verilog +/" SYN_SHARE_DIR "/FF_MODELS/dff.v");
     run("read_verilog +/" SYN_SHARE_DIR "/FF_MODELS/dffe.v");
     run("read_verilog +/" SYN_SHARE_DIR "/FF_MODELS/dffr.v");
     run("read_verilog +/" SYN_SHARE_DIR "/FF_MODELS/dffs.v");
     run("read_verilog +/" SYN_SHARE_DIR "/FF_MODELS/dffrs.v");
     run("read_verilog +/" SYN_SHARE_DIR "/FF_MODELS/dffer.v");
     run("read_verilog +/" SYN_SHARE_DIR "/FF_MODELS/dffes.v");
     run("read_verilog +/" SYN_SHARE_DIR "/FF_MODELS/dffers.v");

     run("blackbox dff dffe dffr dffs dffrs dffer dffes dffers");
  }

  // -------------------------
  // may_wait 
  // -------------------------
  void may_wait ()
  {  
     if (wait) {
       getchar();
     }
  }

  // -------------------------
  // clean_design 
  // -------------------------
  void clean_design(int use_dff_bb_models)
  {
     if (obs_clean) {

        run("splitcells");

        run("splitnets");

	if (use_dff_bb_models) {

	  // Load black box models to get IOs directions for 
	  // 'obs_clean'
	  //
          load_dff_bb_models();
	}

        run("obs_clean");

        run("hierarchy");

     } else {

        run("opt_clean");
     }
  }

  // -------------------------
  // abc_synthesize 
  // -------------------------
  void abc_synthesize()
  {
    if (!G_design) {
       log_warning("Design seems empty !\n");
       return;
    }

    if (opt == "") {
       log_header(G_design, "Performing OFFICIAL PLATYPUS optimization\n");
       run("abc -lut " + sc_syn_lut_size);
       return;
    }

    string mode = opt;

    int nb_cells = (G_design->top_module()->cells()).size();

    // Switch to FAST ABC synthesis script for huge designs in order to avoid
    // runtime blow up.
    //
    // ex: 'ifu', 'l2c', 'e203' designs will be impacted with sometime slight max
    // level degradation but with nice speed-up (ex: 'e203' from 5000 sec. downto
    // 1400 sec.)
    //
    if (nb_cells >= 500000) {

      mode  = "fast"; 

      log_warning("Optimization script changed from '%s' to '%s' due to design size (%d cells)\n",
                  opt.c_str(), mode.c_str(), nb_cells);
      //getchar();
    }

    // Otherwise specific ABC script based flow
    //
    string abc_script = "+/" SYN_SHARE_DIR "/ABC_SCRIPTS/LUT" + sc_syn_lut_size +
	                "/" + abc_script_version + "/" + mode + "_lut" + 
			sc_syn_lut_size + ".scr";

    log_header(G_design, "Calling ABC script in '%s' mode\n", mode.c_str());

    run("abc -script " + abc_script);
  }

  // -------------------------
  // legalize_flops
  // -------------------------
  // Code picked up from procedure 'legalize_flops' in TCL 
  // 'sc_synth_fpga.tcl' (Thierry)
  // DFF features considered so far : 
  //     - enable
  //     - async_set
  //     - async_reset
  //
  void legalize_flops ()
  {

    // Consider all feature combinations 'enable" x "async_set' x 
    // 'async_reset' when features are supported or not : 2x2x2 = 8 
    // combinations to handle.
    //
    
    // 1.
    //
    if (dff_enable && dff_async_set && dff_async_reset) {
      log("Legalize list: $_DFF_P_ $_DFF_PN?_ $_DFFE_PP_ $_DFFE_PN?P_ $_DFFSR_PNN_ $_DFFSRE_PNNP_\n");
      run("dfflegalize -cell $_DFF_P_ 01 -cell $_DFF_PN?_ 01 -cell $_DFFE_PP_ 01 -cell $_DFFE_PN?P_ 01 -cell $_DFFSR_PNN_ 01 -cell $_DFFSRE_PNNP_ 01");
      return;
    }

    // 2.
    //
    if (dff_enable && dff_async_set) {
      log("Legalize list: $_DFF_P_ $_DFF_PN1_ $_DFFE_PP_ $_DFFE_PN1P_\n");
      run("dfflegalize -cell $_DFF_P_ 01 -cell $_DFF_PN1_ 01 -cell $_DFFE_PP_ 01 -cell $_DFFE_PN1P_ 01");
      return;
    }

    // 3.
    //
    if (dff_enable && dff_async_reset) {
      log("Legalize list: $_DFF_P_ $_DFF_PN0_ $_DFFE_PP_ $_DFFE_PN0P_\n");
      run("dfflegalize -cell $_DFF_P_ 01 -cell $_DFF_PN0_ 01 -cell $_DFFE_PP_ 01 -cell $_DFFE_PN0P_ 01");
      return;
    }

    // 4.
    //
    if (dff_enable) {
      log("Legalize list: $_DFF_P_ $_DFF_P??_ $_DFFE_PP_ $_DFFE_P??P_\n");
      run("dfflegalize -cell $_DFF_P_ 01 -cell $_DFF_P??_ 01 -cell $_DFFE_PP_ 01 -cell $_DFFE_P??P_ 01");
      return;
    }

    // 5.
    //
    if (dff_async_set && dff_async_reset) {
      log("Legalize list: $_DFF_P_ $_DFF_PN?_ $_DFFSR_PNN_\n");
      run("dfflegalize -cell $_DFF_P_ 01 -cell $_DFF_PN?_ 01 -cell $_DFFSR_PNN_ 01");
      return;
    }

    // 6.
    //
    if (dff_async_set) {
      log("Legalize list: $_DFF_P_ $_DFF_PN1_\n");
      run("dfflegalize -cell $_DFF_P_ 01 -cell $_DFF_PN1_ 01");
      return;
    }

    // 7.
    //
    if (dff_async_reset) {
      log("Legalize list: $_DFF_P_ $_DFF_PN0_\n");
      run("dfflegalize -cell $_DFF_P_ 01 -cell $_DFF_PN0_ 01");
      return;
    }

    // 8.
    //
    // case of all features are not supported
    //
    // Choose to legalize to async resets even though they
    // won't tech map.  Goal is to get the user to fix
    // their code and put in synchronous resets

    log_warning("No DFF features are suported !\n");
    log_warning("Still Legalize list: $_DFF_P_ $_DFF_P??_\n");
    run("dfflegalize -cell $_DFF_P_ 01 -cell $_DFF_P??_ 01");

  }

  // -------------------------
  // infer_DSPs
  // -------------------------
  void infer_DSPs()
  {

     if (!dsp48) {
       return;
     }

     run("stat");

     run("memory_dff"); // 'dsp' will merge registers, reserve memory port registers first

     // NB: Zero Asic multipliers are signed only
     //

     // Right now in Z1010 part name
     //
     run("techmap -map +/mul2dsp.v -map +/" SYN_SHARE_DIR "/" + part_name + "/DSP/mult18x18_DSP48.v -D DSP_A_MAXWIDTH=18 -D DSP_B_MAXWIDTH=18 "
         "-D DSP_A_MINWIDTH=2 -D DSP_B_MINWIDTH=2 " // Blocks Nx1 multipliers
         "-D DSP_Y_MINWIDTH=9 " // UG901 suggests small multiplies are those 4x4 and smaller
         "-D DSP_SIGNEDONLY=1 -D DSP_NAME=$__MUL18X18");

     run("select a:mul2dsp");
     run("setattr -unset mul2dsp");
     run("opt_expr -fine");
     run("wreduce");
     run("select -clear");
     run("dsp -family DSP48");
     run("chtype -set $mul t:$__soft_mul");

     run("stat");
  }

  // -------------------------
  // resynthesize
  // -------------------------
  // Avoid the heavy synthesis flow and performs a light structural synthesis
  // because we are re-optimizing and re-mapping a netlist.
  //
  void resynthesize() 
  {
    run("stat");

    run("proc");

    run("flatten");

#if 0
    run("opt_expr");
    run("opt_clean");
    run("check");
    run("opt -nodffe -nosdff");
    run("fsm");
    run("opt");
    run("wreduce");
    run("peepopt");
    run("opt_clean");
    run("share");
    run("techmap -map +/cmp2lut.v -D LUT_WIDTH=4");
    run("opt_expr");
    run("opt_clean");

    run("alumacc");
    run("opt");
    run("memory -nomap");

    run("design -save copy");

    run("opt -full");
#endif
    
    run("techmap -map +/techmap.v");

#if 0
    run("memory_map");

    run("demuxmap");
    run("simplemap");
#endif

    run("techmap");

    run("opt -fast");
    run("opt_clean");

    run("opt -full");

    legalize_flops ();

    string sc_syn_flop_library = stringf("+/" SYN_SHARE_DIR "/%s/techlib/tech_flops.v",
                                         part_name.c_str());
    run("techmap -map " + sc_syn_flop_library);

    run("techmap");
    run("opt -purge");

    if (insbuf) {
      run("insbuf");
    }

    clean_design(1);

    run("stat");
    abc_synthesize();

    run("setundef -zero");
    run("clean -purge");

    run("stat");
  }

  // -------------------------
  // help
  // -------------------------
  //
  void help() override
  {
	//   |---v---|---v---|---v---|---v---|---v---|---v---|---v---|---v---|---v---|---v---|
	log("\n");
	log("    synth_fpga [options]\n");
	log("\n");
	log("This command runs Zero Asic FPGA synthesis flow.\n");
	log("\n");
	log("    -top <module>\n");
	log("        use the specified module as top module\n");
        log("\n");

        log("    -no_flatten\n");
        log("        skip flatening. By default, design is flatened.\n");
        log("\n");

        log("    -opt\n");
        log("        specifies the optimization target : area, delay, default, fast.\n");
        log("\n");

        log("    -partname\n");
        log("        Specifies the Architecture partname used. By default it is Z1000.\n");
        log("\n");

        log("    -use_DSP48\n");
        log("        Invoke DSP48 inference. It is off by default.\n");
        log("\n");

        log("    -resynthesis\n");
        log("        switch synthesis flow to resynthesis mode which means a lighter flow.\n");
        log("        It can be used only after performing a first 'synth_fpga' synthesis pass \n");
        log("\n");

        log("    -insbuf\n");
        log("        performs buffers insertion (off by default).\n");
        log("\n");

        log("    -autoname\n");
        log("        Generate, if possible, better wire and cells names close to RTL names rather than\n");
        log("        $abc generic names.\n");
        log("\n");

	// DFF related options
	//
        log("    -no_dff_enable\n");
        log("        specifies that DFF with enable feature is not supported. By default,\n");
        log("        DFF with enable is supported.\n");
        log("\n");
        log("    -no_dff_async_set\n");
        log("        specifies that DFF with asynchronous set feature is not supported. By default,\n");
        log("        DFF with asynchronous set is supported.\n");
        log("\n");
        log("    -no_dff_async_reset\n");
        log("        specifies that DFF with asynchronous reset feature is not supported. By default,\n");
        log("        DFF with asynchronous reset is supported.\n");
        log("\n");
        log("    -seq_opt\n");
        log("        Performs SAT-based sequential optimizations. This is off by default.\n");
        log("\n");

        log("    -obs_clean\n");
        log("        specifies to use 'obs_clean' cleanup function instead of regular \n");
        log("        'opt_clean'.\n");
        log("\n");

        log("    -lut_size\n");
        log("        specifies lut size. By default lut size is 4.\n");
        log("\n");

        log("    -verilog <file>\n");
        log("        write the design to the specified Verilog netlist file. writing of an\n");
        log("        output file is omitted if this parameter is not specified.\n");
	log("\n");

        log("    -show_max_level\n");
        log("        Show longest paths.\n");
        log("\n");

        log("    -csv\n");
        log("        Dump a 'stat.csv' file.\n");
        log("\n");

        log("    -wait\n");
        log("        wait after each 'stat' report for user to touch <enter> key. Help for \n");
        log("        flow analysis/debug.\n");
        log("\n");

	log("The following Yosys commands are executed underneath by 'synth_fpga' :\n");

	help_script();
	log("\n");
  }

  // -------------------------
  // clear_flags
  // -------------------------
  // set default values for global parameters
  //
  void clear_flags() override
  {
	top_opt = "-auto-top";
	opt = "";

	part_name = "Z1000";

	no_flatten = false;
	seq_opt = false;
	autoname = false;
	dsp48 = false;
	resynthesis = false;
	show_max_level = false;
	csv = false;
	insbuf = false;

	wait = false;

	dff_enable = true;
	dff_async_set = true;
	dff_async_reset = true;

	obs_clean = false;

	verilog_file = "";

	abc_script_version = "BEST";

	sc_syn_lut_size = "4";
  }

  // -------------------------
  // execute
  // -------------------------
  //
  void execute(std::vector<std::string> args, RTLIL::Design *design) override
  {
	string run_from, run_to;
	clear_flags();

        log_header(design, "Executing 'synth_fpga'\n\n");

	G_design = design;

	size_t argidx;

	for (argidx = 1; argidx < args.size(); argidx++)
        {
          if (args[argidx] == "-top" && argidx+1 < args.size()) {
 	     top_opt = "-top " + args[++argidx];
	     continue;
	  }

	  if (args[argidx] == "-opt" && argidx+1 < args.size()) {
	     opt = args[++argidx];
	     if (opt_options.count(opt) == 0) {
                log_cmd_error("-opt option '%s' is unknown. Please see help.\n", 
                              (args[argidx]).c_str());
	     }
	     continue;
          }

          if (args[argidx] == "-resynthesis") {
             resynthesis = true;
             continue;
          }

          if (args[argidx] == "-no_flatten") {
             no_flatten = true;
             continue;
          }

          if (args[argidx] == "-seq_opt") {
             seq_opt = true;
             continue;
          }

          if (args[argidx] == "-use_DSP48") {
             dsp48 = true;
             continue;
          }

          if (args[argidx] == "-insbuf") {
             insbuf = true;
             continue;
          }

          if (args[argidx] == "-autoname") {
             autoname = true;
             continue;
          }

          if (args[argidx] == "-partname" && argidx+1 < args.size()) {
             part_name = args[++argidx];
	     if (partnames.count(part_name) == 0) {
                log("ERROR: -partname '%s' is unknown.\n", (args[argidx]).c_str());
		log("       List of available partnames is :\n");
		for (auto part_name : partnames) {
                   log ("               - %s\n", part_name.c_str());
		}
                log_error("Please choose a correct partname within the list.\n");
	     }
             continue;
          }

	  if (args[argidx] == "-lut_size" && argidx+1 < args.size()) {
             sc_syn_lut_size = args[++argidx];
             continue;
          }

	  if (args[argidx] == "-abc_script_version" && argidx+1 < args.size()) {
             abc_script_version = args[++argidx];
             continue;
          }

	  if (args[argidx] == "-obs_clean") {
             obs_clean = true;
             continue;
          }

	  if (args[argidx] == "-verilog" && argidx+1 < args.size()) {
             verilog_file = args[++argidx];
             continue;
          }

          // Support of DFF features : with or without 
          //     - enable
          //     - async set
          //     - async reset
          //
          if (args[argidx] == "-no_dff_enable") {
             dff_enable = false;
             continue;
          }

          if (args[argidx] == "-no_dff_async_set") {
             dff_async_set = false;
             continue;
          }

          if (args[argidx] == "-no_dff_async_reset") {
             dff_async_reset = false;
             continue;
          }

          if (args[argidx] == "-show_max_level") {
             show_max_level = true;
             continue;
          }

          if (args[argidx] == "-csv") {
             csv = true;
             continue;
          }

          // for debug, flow analysis
          //
	  if (args[argidx] == "-wait") {
             wait = true;
             continue;
          }

          log_cmd_error("Unknown option : %s\n", (args[argidx]).c_str());
	}
        extra_args(args, argidx, design);

        if (!design->full_selection()) {
           log_cmd_error("This command only operates on fully selected designs!\n");
        }

        log_header(design, "Executing Zero Asic 'synth_fpga' flow.\n");
        log_push();

        run_script(design, run_from, run_to);

        log_pop();
  }

  // ---------------------------------------------------------------------------
  // script (synth_fpga flow) 
  // ---------------------------------------------------------------------------
  //
  // VERSION 1.0 (05/13/2025, Thierry): 
  //
  //        - as a starter, we mimic what is done in : 
  //          '.../siliconcompiler/tools/yosys/sc_synth_fpga.tcl' 
  //
  //        - we try to handle DFF legalization by taking care of DFF features
  //        support like handling DFF with 'enable', 'async_set', 'async_reset'.
  //
  //        - other encapsulated code with #if 0 coming from 'sc_synth_fpga.tcl'
  //        needs to be handled in this 'synth_fpga' command.
  //
  // ---------------------------------------------------------------------------
  void script() override
  {

    if (!G_design) {
       log_warning("Design seems empty !\n");
       return;
    }

    auto startTime = std::chrono::high_resolution_clock::now();

    log("\nPLATYPUS flow using 'synth_fpga' Yosys plugin command\n");

    log("'Zero Asic' FPGA Synthesis Version : %s\n", SYNTH_FPGA_VERSION);

#if 0
    // TCL scipt version of PLATYPUS
    //

    # Pre-processing step:  if DSPs instance are hard-coded into
    # the users design, we can use a blackbox flow for DSP mapping
    # as follows:

    if { [sc_cfg_exists fpga $sc_partname file yosys_macrolib] } {
        set sc_syn_macrolibs \
            [sc_cfg_get fpga $sc_partname file yosys_macrolib]

        foreach macrolib $sc_syn_macrolibs {
            yosys read_verilog -lib $macrolib
        }
    }
#endif

    // Extra line added versus 'sc_synth_fpga.tcl' tcl script version
    //
    run(stringf("hierarchy -check %s", help_mode ? "-top <top>" : top_opt.c_str()));

    Module* topModule = G_design->top_module();

    if (!topModule) {
       log_warning("Design seems empty !\n");
       return;
    }

    if (resynthesis) {

       resynthesize();

       return;
    }

    run("proc");

    // Print stat when design is flatened. We flatened a copy, stat the copy
    // and get back to the original hierarchical design.
    //
#if 0
    run("design -save original");
    run("flatten");

    log("\n# ------------------------- \n");
    log("#  Design first statistics  \n");
    log("# ------------------------- \n");
    run("stat");
    run("design -load original");
#endif

    may_wait();

    if (!no_flatten) {
      run("flatten");
    }

    // Note there are two possibilities for how macro mapping might be done:
    // using the extract command (to pattern match user RTL against
    // the techmap) or using the techmap command.  The latter is better
    // for mapping simple multipliers; the former is better (for now)
    // for mapping more complex DSP blocks (MAC, pipelined blocks, etc).
    // and is also more easily extensible to arbitrary hard macros.
    // Run separate passes of both to get best of both worlds

    // An extract pass needs to happen prior to other optimizations,
    // otherwise yosys can transform its internal model into something
    // that doesn't match the patterns defined in the extract library

#if 0
    // TCL scipt version of PLATYPUS
    //
    
    if { [sc_cfg_exists fpga $sc_partname file yosys_extractlib] } {
        set sc_syn_extractlibs \
            [sc_cfg_get fpga $sc_partname file yosys_extractlib]

        foreach extractlib $sc_syn_extractlibs {
            yosys log "Run extract with $extractlib"
            yosys extract -map $extractlib
        }
    }
#endif

    // Other hard macro passes can happen after the generic optimization
    // passes take place.

    // Generic optimization passes; this is a fusion of the VTR reference
    // flow and the Yosys synth_ice40 flow
    //
    run("opt_expr");
    run("opt_clean");
    run("check");
    run("opt -nodffe -nosdff");
    run("fsm");
    run("opt");
    run("wreduce");
    run("peepopt");
    run("opt_clean");
    run("share");
    run("techmap -map +/cmp2lut.v -D LUT_WIDTH=" + sc_syn_lut_size);
    run("opt_expr");
    run("opt_clean");

    // Extra line added versus 'sc_synth_fpga.tcl' tcl script version
    //
    run("stat");

    may_wait();

    // Here is a remaining customization pass for DSP tech mapping

    // Map DSP blocks before doing anything else,
    // so that we don't convert any math blocks
    // into other primitives
    //

#if 0
    // TCL scipt version of PLATYPUS
    //

    if { [sc_cfg_exists fpga $sc_partname file yosys_dsp_techmap] } {
        set sc_syn_dsp_library \
            [sc_cfg_get fpga $sc_partname file yosys_dsp_techmap]

        yosys log "Run techmap flow for DSP Blocks"
        set formatted_dsp_options [get_dsp_options $sc_syn_dsp_options]
        yosys techmap -map +/mul2dsp.v -map $sc_syn_dsp_library \
            {*}$formatted_dsp_options

        post_techmap
    }
#endif

    // Map DSP components
    //
    infer_DSPs();

    // Mimic ICE40 flow by running an alumacc and memory -nomap passes
    // after DSP mapping
    //  
    run("alumacc");
    run("opt");
    run("memory -nomap");

    run("design -save copy");

    // First strategy : we deeply optimize logic but we may break its
    // nice structure than can map in nice DFF enable 
    // (ex: big_designs/VexRiscv).
    // But it may help for some designs (ex: medium_designs/xtea)
    //
    run("opt -full");
    
    run("techmap -map +/techmap.v");

#if 0
    // TCL scipt version of PLATYPUS
    //

    set sc_syn_memory_libmap ""
    if { [sc_cfg_exists fpga $sc_partname file yosys_memory_libmap] } {
        set sc_syn_memory_libmap \
            [sc_cfg_get fpga $sc_partname file yosys_memory_libmap]
    }
    set sc_do_rom_map [expr { [lsearch -exact $sc_syn_feature_set mem_init] < 0 }]
    set sc_syn_memory_library ""
    if { [sc_cfg_exists fpga $sc_partname file yosys_memory_techmap] } {
        set sc_syn_memory_library \
            [sc_cfg_get fpga $sc_partname file yosys_memory_techmap]
    }

    if { [sc_map_memory $sc_syn_memory_libmap $sc_syn_memory_library $sc_do_rom_map] } {
        post_techmap
    }
#endif

    // After doing memory mapping, turn any remaining
    // $mem_v2 instances into flop arrays
    //
    run("memory_map");

    run("demuxmap");
    run("simplemap");

    // Call the zero asic version of 'opt_dff', e.g 'zopt_dff', especially 
    // taking care of the -sat option.
    //
    if (seq_opt) {
      run("stat");
      run("zopt_dff -sat");
    }

    // Extra lines that help to win Area (ex: vga_lcd from 31K Lut4 downto 14.8K)
    //
    // IMPROVE-2
    //
    run("techmap");

#if 0
    log("opt -fast");
    run("opt -fast");

    run("opt_clean");
    // END IMPROVE-2
#endif

    // original TCL call : legalize_flops $sc_syn_feature_set
    //
    log("opt -full");
    run("opt -full");

    legalize_flops (); // C++ version of TCL call

#if 0
    // TCL scipt version of PLATYPUS
    //

    if { [sc_cfg_exists fpga $sc_partname file yosys_flop_techmap] } {
        set sc_syn_flop_library \
            [sc_cfg_get fpga $sc_partname file yosys_flop_techmap]
        yosys techmap -map $sc_syn_flop_library

        post_techmap
    }

#else
    
    // C++ Version
    //
    // Map on the DFF of the architecture (partname)
    //
    string sc_syn_flop_library = stringf("+/" SYN_SHARE_DIR "/%s/techlib/tech_flops.v",
		                         part_name.c_str());
    run("techmap -map " + sc_syn_flop_library);


#if 0
    run("design -save first_solution");
    run("stat");
    int nb_cells1 = (G_design->top_module()->cells()).size();
    log("**  First solution : %d\n", nb_cells1);


    run("design -load copy");
    
    // Second strategy : we simply clean logic so that we keep its 
    // nice structure than can map in nice DFF enable 
    // (ex: big_designs/VexRiscv).
    //
    run("opt_clean");

    run("techmap -map +/techmap.v");

    run("memory_map");

    run("demuxmap");
    run("simplemap");

    run("techmap");
    run("opt -fast");
    run("opt_clean");

    run("opt -full");

    legalize_flops (); // C++ version of TCL call

    run("techmap -map " + sc_syn_flop_library);

    run("stat");
    int nb_cells2 = (G_design->top_module()->cells()).size();

    log("**  First solution  : %d cells\n", nb_cells1);
    log("**  Second solution : %d cells\n", nb_cells2);

    //getchar();

    // Choose best solution between the two strategies
    //
    if (1 && (nb_cells1 < nb_cells2)) {
       run("design -load first_solution");
    }
#endif

    // 'post_techmap' without arguments gives the following 
    // according to '.../siliconcompiler/tools/yosys/procs.tcl'
    // IMPROVE-1
    //
    run("techmap");
    run("opt -purge");
    // END IMPROVE-1
#endif

    // Perform preliminary buffer insertion before passing to ABC to help reduce
    // the overhead of final buffer insertion downstream
    //
    if (insbuf) {
      run("insbuf");
    }

    run("stat");

    may_wait();

    clean_design(1);

    run("stat");

    may_wait();

    // Optimize and map through ABC the combinational logic part of the design.
    //
    run("stat");
    abc_synthesize();

    run("stat");

    may_wait();

    run("setundef -zero");
    run("clean -purge");

    // tries to give public names instead of using $abc generic names.
    // Right now this procedure blows up runtime for medium/big designs
    //
    if (autoname) {
      run("autoname");
    }

    if (!verilog_file.empty()) {
       log("Dump Verilog file '%s'\n", verilog_file.c_str()); 
       run(stringf("write_verilog -noexpr -nohex -nodec %s", verilog_file.c_str()));
    }

    run("stat");

    auto endTime = std::chrono::high_resolution_clock::now();
    auto elapsed = std::chrono::duration_cast<std::chrono::nanoseconds>(endTime - startTime);

    float totalTime = 1 + elapsed.count() * 1e-9;

    log("   PartName : %s\n", part_name.c_str());
    log("\n");
    log("   'Zero Asic' FPGA Synthesis Version : %s\n", SYNTH_FPGA_VERSION);
    log("\n");
    log("   Total Synthesis Run Time = %.1f sec.\n", totalTime);

    // Show longest path in 'delay' mode
    //
    if (show_max_level) {
      run("max_level"); // -> store 'maxlvl' in scratchpad with 'max_level.max_levels'
    }

    if (csv) {
       dump_csv_file("stat.csv", (int)totalTime);
    }

    run(stringf("write_verilog -noexpr -nohex -nodec %s", "netlist_synth_fpga.verilog"));

  } // end script()

} SynthFpgaPass;

PRIVATE_NAMESPACE_END
