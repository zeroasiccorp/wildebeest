#include "kernel/yosys.h"
#include "kernel/celltypes.h"
#include "kernel/sigtools.h"
#include <cassert>
#include <chrono>
#include <iostream>
#include <fstream>

USING_YOSYS_NAMESPACE
PRIVATE_NAMESPACE_BEGIN

static bool dot = false;

struct MaxHeigthWorker
{
   RTLIL::Design *design;
   RTLIL::Module *module;
   SigMap sigmap;

   // Bit info attached to a SigBit:
   //    - level
   //    - sigBit
   //    - driven cell from the sigbit
   //    - heigth
   //
   dict<SigBit, tuple<pool<SigBit>*, Cell*, int>> bits;

#define INPUTS(x) get<0>(x)
#define CELL(x)   get<1>(x)
#define HEIGTH(x) get<2>(x)

   pool<SigBit> visited_bits;

   pool<SigBit> cps;
   pool<SigBit> luts;
   dict<SigBit, int> fanout;

   // ---------------------
   // MaxHeigthWorker
   // ---------------------
   //
   MaxHeigthWorker(RTLIL::Module *module) : design(module->design), module(module),
                                            sigmap(module)
   {

      for (auto wire : module->selected_wires()) {

         for (auto bit : sigmap(wire)) {

            bits[bit] = tuple<pool<SigBit>*, Cell*, int>(nullptr, nullptr, -1);
         }
      }

      for (auto cell : module->selected_cells()) {

         if (cell->type != "$lut") {
           continue;
	 }

	 SigBit out;
	 bool out_exists = false;

	 pool<SigBit>* inputs = new pool<SigBit>;

         for (auto &conn : cell->connections()) {

             for (auto bit : sigmap(conn.second)) {

                 if (cell->input(conn.first)) {
                    inputs->insert(bit);
		    continue;
                 }

                 if (cell->output(conn.first)) {
                    out = bit;
	            out_exists = true;
                 }
             }
         }

	 if (!out_exists) {
            log_warning("It seems Lut cell '%s' is undriven.\n", log_id(cell->name));
	    continue;
	 } 

	 luts.insert(out);

         auto &bitinfo = bits.at(out);

         INPUTS(bitinfo) = inputs;
         CELL(bitinfo) = cell;
      }
   }
   
   // ---------------------
   // get_cp_logic_rec
   // ---------------------
   void get_cp_logic_rec(SigBit bit, int heigth)
   {
     auto &bitinfo = bits.at(bit);

     // Bit already in 'cps' so already traversed and TFI already added in 'cps'
     //
     if (cps.count(bit)) {
        return;
     }

     // return if a PI
     //
     if (HEIGTH(bitinfo) == 0) {
        return;
     }

     // If on the CP then add it
     //
     if (HEIGTH(bitinfo) == heigth) {
        assert(CELL(bitinfo)); // make sure it is driven
        cps.insert(bit);
     }

     // return if not on the CP
     //
     if (HEIGTH(bitinfo) < heigth) {
        return;
     }

     if (HEIGTH(bitinfo) > heigth) {
       log_error("Problem in the CP extractor : Height %d must be less than height %d\n", 
		 HEIGTH(bitinfo), heigth);
     }

     pool<SigBit>* inputs = INPUTS(bitinfo);

     for (auto from : *inputs) {
	     
       // Collect recursivelet from the TFI from 'from' with heigth 'heigth-1'
       //
       get_cp_logic_rec(from, heigth-1);
     }
   
   }


   // ---------------------
   // get_cp_logic
   // ---------------------
   void get_cp_logic(int max_heigth)
   {
     // Extract CP from the 'bits' starting with 'max_height'.
     //
     for (auto &it : bits) {

        if (HEIGTH(it.second) == max_heigth) {
          get_cp_logic_rec(it.first, max_heigth);
	}
     }
   }

   // ---------------------
   // legalize_dot_name
   // ---------------------
   void legalize_dot_name(string& s)
   {
     for (long unsigned int i=0; i<s.size(); i++) {

        if (s[i] == '$') {
           s[i] = '_';
           continue;
	}
        if (s[i] == '\\') {
           s[i] = '_';
           continue;
	}
        if (s[i] == '[') {
           s[i] = '_';
           continue;
	}
        if (s[i] == ']') {
           s[i] = '_';
           continue;
	}
        if (s[i] == '.') {
           s[i] = '_';
           continue;
	}
        if (s[i] == ':') {
           s[i] = '_';
           continue;
	}
     }
   }

   // ---------------------
   // dot_cell
   // ---------------------
   void dot_cell(std::ofstream& cells_dot, Cell* cell)
   {
      for (auto &conn : cell->connections()) {

         string cell_name = log_id(cell->name);
	 legalize_dot_name(cell_name);

         for (auto bit : sigmap(conn.second)) {

	    string name = log_signal(bit);
	    legalize_dot_name(name);

            if (cell->input(conn.first)) {
	        cells_dot << cell_name << " -> " << name << "\n"; 
		continue;
            }

            if (cell->output(conn.first)) {
	        cells_dot << name << " -> " << cell_name << "\n"; 
		continue;
            }
        }
     }
   }


   // ---------------------
   // dump_cp_dot
   // ---------------------
   void dump_cp_dot()
   {
     std::ofstream cells_dot;

     cells_dot.open ("cp.dot");

     cells_dot << "digraph cp {\n";

     // Dump definition and collect Cells in CP
     //
     pool<Cell*> cells;

     for (auto bit : cps) {

        auto &bitinfo = bits.at(bit);

        Cell* cell = CELL(bitinfo);
        int heigth= HEIGTH(bitinfo);
        pool<SigBit>* inputs = INPUTS(bitinfo);

        cells.insert(cell);

        string cell_name = log_id(cell->name);
	legalize_dot_name(cell_name);
	int fo = fanout[bit];
	long unsigned int nb = inputs->size();
        cells_dot << cell_name << " [shape=box, label=\"" << cell_name << "\nLUT" << nb << "\nheigth=" << heigth << "\nfo=" << fo << "\"]\n"; 
     }

     // Dump definition and Collect cells periphery bits
     //
     pool<SigBit> cp_bits;

     for (auto cell : cells) {

       for (auto &conn : cell->connections()) {

          for (auto bit : sigmap(conn.second)) {

            cp_bits.insert(bit);

	    string name = log_signal(bit);
	    legalize_dot_name(name);

            auto &bitinfo = bits.at(bit);
            int heigth= HEIGTH(bitinfo);

            if (cps.count(bit)) {
              cells_dot << name << " [color=\"red\" style=filled label=\"" << name << "\nheigth=" << heigth << "\"]\n"; 
	    } else {
              cells_dot << name << " [color=\"green\" style=filled label=\"" << name << "\nheigth=" << heigth << "\"]\n"; 
	    }
         }
       }
     }

     for (auto cell : cells) {
        dot_cell(cells_dot, cell);
     }

     cells_dot << "}\n";
     cells_dot.close ();
   }


   // ---------------------
   // get_heigth_rec
   // ---------------------
   int get_heigth_rec(SigBit bit)
   {
     auto &bitinfo = bits.at(bit);

     fanout[bit] += 1;

     int heigth = HEIGTH(bitinfo);

     if (heigth >= 0) {
        return heigth;
     }

     if (!CELL(bitinfo)) { // if 'bit' has no cell driving it it is a PI or undriven wire
       HEIGTH(bitinfo) = 0;
       return 0;
     }

     if (visited_bits.count(bit) > 0) {
        HEIGTH(bitinfo) = 0;
        log_warning("Detected loop at %s in %s\n", log_signal(bit), log_id(module));
        return 0;
     }

     visited_bits.insert(bit);

     pool<SigBit>* inputs = INPUTS(bitinfo);

     int max_heigth = -1;

     for (auto in : *inputs) {

         int in_heigth = get_heigth_rec(in);

	 if (in_heigth > max_heigth) {
            max_heigth = in_heigth;
	 }
     }

     HEIGTH(bitinfo) = max_heigth+1;

     return max_heigth+1;
   }

   // ---------------------
   // get_max_height
   // ---------------------
   int get_max_height()
   {
     int max_heigth = -1;

     for (auto &it : bits) {
       fanout[it.first] = 0;
     }

     for (auto &it : bits) {

        if (HEIGTH(it.second) < 0) { // if Heigth not computed yet

           visited_bits.clear();

           int heigth = get_heigth_rec(it.first);

           if (heigth > max_heigth) {
              max_heigth = heigth;
           }
        }
     }

     return max_heigth;
   }

   // ---------------------
   // run
   // ---------------------
   void run()
   {
     auto startTime = std::chrono::high_resolution_clock::now();

     // get the max heigth of the whole LUT logic
     //
     int max_heigth = get_max_height();

     design->scratchpad_set_int("max_heigth.max_heigth", max_heigth);

     // get the logic on the critical paths starting with heigth 'max_heigth'
     //
     get_cp_logic(max_heigth);

     // Eventually dump the dot file fo the CP logic
     //
     if (dot) {
       dump_cp_dot();
     }

     log("\n");
     log("   Max heigth / Max levels    = %d\n", max_heigth);
     log("   CP bits size               = %ld\n", cps.size());
     log("   Total lut output bits      = %ld\n", luts.size());

     auto endTime = std::chrono::high_resolution_clock::now();
     auto elapsed = std::chrono::duration_cast<std::chrono::nanoseconds>(endTime - startTime);

     float totalTime = 1 + elapsed.count() * 1e-9;

     log("   [Run Time = %.1f sec.]\n", totalTime);

   }
};

struct MaxHeigthPass : public ScriptPass {

   RTLIL::Design *G_design = NULL; 

   MaxHeigthPass() : ScriptPass("max_heigth", "print max logic heigth") { }

   // ---------------------
   // help
   // ---------------------
   void help() override
   {
      //   |---v---|---v---|---v---|---v---|---v---|---v---|---v---|---v---|---v---|---v---|
      log("\n");
      log("    max_heigth [options] \n");
      log("\n");
      log("This command prints the max logic height in the design. (Only considers\n");
      log("paths within a single module, so the design must be flattened to get the)\n");
      log("overall longest path in the design.\n");
      log("\n");
      log("    -dot\n");
      log("        Dump a 'dot' file representing only the critical logic.\n");
      log("\n");
   }

   // ---------------------
   // clear_flags
   // ---------------------
   void clear_flags() override
   {
     dot = false;
   }

   // ---------------------
   // execute
   // ---------------------
   void execute(std::vector<std::string> args, RTLIL::Design *design) override
   {
     string run_from, run_to;

     clear_flags();

     log_header(design, "Executing 'max_heigth' command (find max logic level).\n");

     size_t argidx;

     G_design = design;

     for (argidx = 1; argidx < args.size(); argidx++) {
           if (args[argidx] == "-dot") {
             dot = true;
             continue;
        }
     }

     extra_args(args, argidx, design);

     run_script(design, run_from, run_to);

   }

   void script() override
   {
     if (!G_design) {
       log_warning("Design seems empty !\n");
       return;
     }

     if (dot) {

       run("design -push-copy");

       run("splitnets -ports");

       run(stringf("write_verilog -noexpr -nohex -nodec -norename -simple-lhs dot_cells.vlg"));
       run(stringf("write_verilog -nohex -nodec -norename -simple-lhs dot_expr.vlg"));
     }

     for (Module *module : G_design->selected_modules())
     {
       if (module->has_processes_warn()) {
          continue;
       }

       MaxHeigthWorker worker(module);
       worker.run();
     }

     if (dot) {
       run("design -pop");
     }

   }

} MaxHeigthPass;

PRIVATE_NAMESPACE_END
