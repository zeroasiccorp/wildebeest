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
#include <chrono>
#include <iomanip>

#define SYNTH_FPGA_VERSION "1.0"

USING_YOSYS_NAMESPACE
PRIVATE_NAMESPACE_BEGIN

struct ReportStatPass : public ScriptPass
{
  // Global data
  //
  RTLIL::Design *G_design = NULL; 
  string csv_stat_file;

  // Methods
  //
  ReportStatPass() : ScriptPass("report_stat", "Dump flattened netlist stats in file 'stat.csv'") { }

  // -------------------------
  // getNumberOfDffs
  // -------------------------
  int getNumberOfLuts() {

     int nb = 0;

     for (auto cell : G_design->top_module()->cells()) {

         // Generic Luts like for Zero Asic Z1000
         //
         if (cell->type.in(ID($lut))) {
             nb++;
	     continue;
         }

         // Xilinx 'xc4v' Luts
         //
         if (cell->type.in(ID(LUT1), ID(LUT2), ID(LUT3), ID(LUT4), ID(INV))) {
             nb++;
	     continue;
         }
	 
         // Lattice 'Mach xo2' Luts
         //
         if (cell->type.in(ID(LUT1), ID(LUT2), ID(LUT3), ID(LUT4))) {
             nb++;
	     continue;
         }
	 
         // ice40 'hx' Luts
         //
         if (cell->type.in(ID(SB_LUT4))) {
             nb++;
	     continue;
         }
	 
         // Quicklogic 'pp3' Luts
         //
         if (cell->type.in(ID(LUT1), ID(LUT2), ID(LUT3), ID(LUT4))) {
             nb++;
	     continue;
         }
	 
         // Quicklogic 'pp3' mux4x0
         //
         if (cell->type.in(ID(mux4x0))) {
             nb += 3;  // equivalent to 3 LUT3 (Mux)
	     continue;
         }
	 
         // Quicklogic 'pp3' mux8x0
         //
         if (cell->type.in(ID(mux8x0))) {
             nb += 7;  // equivalent to 7 LUT3 (Mux)
	     continue;
         }
	 
         // Microchip 'polarfire' Luts
         //
         if (cell->type.in(ID(CFG1), ID(CFG2), ID(CFG3), ID(CFG4))) {
             nb++;
	     continue;
         }
	 
         // Intel 'cycloneiv' Luts
         //
         if (cell->type.in(ID($not), ID(cycloneiv_lcell_comb))) {
             nb++;
	     continue;
         }
     }

     return nb;
  }

  // -------------------------
  // getNumberOfDffs
  // -------------------------
  int getNumberOfDffs() {

    int nb = 0;

    for (auto cell : G_design->top_module()->cells()) {

        // Zero Asic Z1000 DFFs
        //
        if (cell->type.in(ID(dff), ID(dffe), ID(dffr), ID(dffer),
                          ID(dffs), ID(dffrs), ID(dffes), ID(dffers))) {
             nb++;
	     continue;
        }

        // Xilinx 'xc4v' DFFs
        //
        if (cell->type.in(ID(FDCE), ID(FDPE), ID(FDRE), ID(FDRE_1),
                          ID(FDSE),
			  ID(LDCE))) { // LATCH !!!
             nb++;
	     continue;
        }
	
        // Lattice 'Mach ox2' DFFs
        //
        if (cell->type.in(ID(TRELLIS_FF))) {
             nb++;
	     continue;
        }
	
        // ice40 'hx' DFFs
        //
        if (cell->type.in(ID(SB_DFF), ID(SB_DFFE), ID(SB_DFFER), ID(SB_DFFESR), 
                          ID(SB_DFFESS), ID(SB_DFFN), ID(SB_DFFR), ID(SB_DFFS), 
			  ID(SB_DFFSR), ID(SB_DFFES))) {
            nb++;
            continue;
        }
	
        // Quicklogic 'pp3' DFFs
        //
        if (cell->type.in(ID(dffepc))) { 
            nb++;
            continue;
        }
	
        // Microchip 'polarfire' Luts
        //
        if (cell->type.in(ID(SLE))) {
            nb++;
	    continue;
        }

        // Intel 'cycloneiv' DFFs
        //
        if (cell->type.in(ID(dffeas))) { 
            nb++;
            continue;
        }
    }

    return nb;
  }


  // -------------------------
  // getNumberOfDSPs
  // -------------------------
  int getNumberOfDSPs() {

    int nb = 0;

    for (auto cell : G_design->top_module()->cells()) {

	// Xilinx xc4v
	//
        if (cell->type.in(ID(DSP48))) {
             nb++;
             continue;
        }
	
	// Xilinx xc5v
	//
        if (cell->type.in(ID(DSP48E))) {
             nb++;
             continue;
        }

	// Ice40
	//
        if (cell->type.in(ID(SB_MAC16))) {
             nb++;
             continue;
        }
    }

    return nb;
  }

  // -------------------------
  // getNumberOfBRAMs
  // -------------------------
  int getNumberOfBRAMs() {

    int nb = 0;

    for (auto cell : G_design->top_module()->cells()) {

        // Xilinx xc4v
        //
        if (cell->type.in(ID(RAMB16))) {
             nb++;
             continue;
        }

        // Ice40
        //
        if (cell->type.in(ID(SB_RAM40_4K))) {
             nb++;
             continue;
        }
	
        // Lattice xo2
        //
        if (cell->type.in(ID(DP8KC))) {
             nb++;
             continue;
        }
	
        // Intel cycloneiv
        //
        if (cell->type.in(ID(altsyncram))) {
             nb++;
             continue;
        }
	
        // Microchip polarfire
        //
        if (cell->type.in(ID(RAM1K20))) {
             nb++;
             continue;
        }
    }

    return nb;
  }


  // -------------------------
  void help() override
  {
	//   |---v---|---v---|---v---|---v---|---v---|---v---|---v---|---v---|---v---|---v---|
	log("\n");
	log("    report_stat\n");
	log("\n");
	log("This command reports stat on a flattened netlist. Data are dumped.\n");
	log("in file 'stat.csv'.\n");
	log("\n");
        log("    -csv <file>\n");
        log("        write design statistics into a CSV file. Default file name\n");
        log("        is 'stat.csv'.\n");
	log("\n");


	help_script();
	log("\n");
  }

  void clear_flags() override
  {
        csv_stat_file = "stat.csv";
  }

  void execute(std::vector<std::string> args, RTLIL::Design *design) override
  {
	string run_from, run_to;
	clear_flags();

	G_design = design;

	size_t argidx;
	for (argidx = 1; argidx < args.size(); argidx++)
	{
	        if (args[argidx] == "-csv" && argidx+1 < args.size()) {
                        csv_stat_file = args[++argidx];
                        continue;
                }
	}
	extra_args(args, argidx, design);

	if (!design->full_selection()) {
           log_cmd_error("This command only operates on fully selected designs!\n");
	}

	log_header(design, "Executing 'report_stat'.\n");
	log_push();

	run_script(design, run_from, run_to);

	log_pop();
  }
  
  // ---------------------------------------------------------------------------
  // report_stat 
  // ---------------------------------------------------------------------------
  void script() override
  {
    Module* topModule = G_design->top_module();

    if (!topModule) {
       log_warning("Design seems empty !\n");
       return;
    }

    string topName = log_id(topModule->name);

    int nbLuts = getNumberOfLuts();

    int nbDffs = getNumberOfDffs();

    int nbDSPs = getNumberOfDSPs();

    int nbBRAMs = getNumberOfBRAMs();

    int maxlvl = -1;

    // call 'max_level' command if not called yet
    //
    run("max_level -noff"); // -> store 'maxlvl' in scratchpad with 'max_level.max_levels'

    maxlvl = G_design->scratchpad_get_int("max_level.max_levels", 0);

    string start = G_design->scratchpad_get_string("time_chrono_start");
    string end =  G_design->scratchpad_get_string("time_chrono_end");

    double d_start = atof(start.c_str());
    double d_end = atof(end.c_str());

    int duration = (int)(d_end - d_start) + 1;

    log("\n");

#if 0
    log("   Start time = %s\n", start.c_str());
    log("   End time   = %s\n", end.c_str());
#endif

    log("   Duration   = %d sec.\n", duration);

    // -----
    // Open the csv file and dump the stats.
    //
    std::ofstream csv_file(csv_stat_file);

    csv_file << topName + ",";
    csv_file << std::to_string(nbLuts) + ",";
    csv_file << std::to_string(nbDffs) + ",";
    csv_file << std::to_string(nbDSPs) + ",";
    csv_file << std::to_string(nbBRAMs) + ",";
    csv_file << std::to_string(maxlvl) + ",";
    csv_file << std::to_string(duration);
    csv_file << std::endl;

    csv_file.close();

    log("\n   Dumped file %s\n", csv_stat_file.c_str());

    run("stat");

  } // end script()

} ReportStatPass;

PRIVATE_NAMESPACE_END
